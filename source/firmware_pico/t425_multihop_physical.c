#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "t425_pio_phy.h"
#include "t425_node_config.h"
#include "t425_reliable_link.h"
#include "t425_router.h"
#include "t425_forward.h"

/*
 * v2.46 physical integration benchmark.
 *
 * Each Pico has the same firmware. NODE_ID selects its identity.
 * The forwarding envelope is carried inside the hop-local DATA payload.
 *
 * Topology:
 *   P00 <-> P01 <-> P02
 *
 * P01 consumes the P00->P01 hop, sends the ACK for that hop, then creates
 * a NEW hop-local DATA frame P01->P02 containing the same forwarding envelope.
 *
 * This is stop-and-wait per hop; no end-to-end ARQ is claimed here.
 */

#define T425_APP_PAYLOAD 8u
#define T425_ENV_LEN ((uint8_t)sizeof(t425_forward_hdr_t))
#define T425_WIRE_PAYLOAD ((uint8_t)(sizeof(t425_forward_hdr_t) + T425_APP_PAYLOAD))

static t425_pio_phy_t phy;
static t425_rl_t rl;
static t425_parser_t parser;
static t425_router_t router;

static uint8_t local_tx_seq = 0;
static uint32_t last_report = 0;
static uint32_t delivered = 0;
static uint32_t forwarded = 0;
static uint32_t dropped = 0;

static void wire_send(const t425_frame_t *f) {
    t425_pio_phy_write(&phy, (const uint8_t *)f,
                       (uint16_t)(T425_OVERHEAD + f->len));
}

static void send_ack(uint8_t dst, uint8_t seq) {
    t425_frame_t a;
    t425_frame_build(&a, T425_TYPE_ACK, T425_NODE_ID, dst, seq, 0, 0);
    wire_send(&a);
}

static bool send_hop(uint8_t next_hop, const uint8_t *payload, uint8_t len) {
    if (len > T425_MAX_PAYLOAD) return false;

    t425_frame_t f;
    t425_frame_build(&f, T425_TYPE_DATA, T425_NODE_ID, next_hop,
                     local_tx_seq++, payload, len);
    wire_send(&f);
    return true;
}

static void make_initial_packet(void) {
    uint8_t payload[T425_WIRE_PAYLOAD];
    t425_forward_hdr_t h;

    t425_forward_build(&h, T425_NODE_ID, 1u, 2u, 4u, T425_APP_PAYLOAD);
    memcpy(payload, &h, sizeof(h));

    for (uint8_t i = 0; i < T425_APP_PAYLOAD; ++i)
        payload[sizeof(h) + i] = (uint8_t)("T425TEST"[i]);

    send_hop(1u, payload, sizeof(payload));
}

static void process_data(const t425_frame_t *f) {
    bool duplicate = false;
    bool deliver = t425_rl_accept_data(&rl, f->seq, &duplicate);

    /* Every valid hop-local DATA gets an ACK, including duplicates. */
    send_ack(f->src, f->seq);

    if (!deliver) return;
    if (f->len < sizeof(t425_forward_hdr_t)) {
        dropped++;
        return;
    }

    t425_forward_hdr_t h;
    memcpy(&h, f->payload, sizeof(h));

    if (h.final_dst == T425_NODE_ID) {
        delivered++;
        printf("DELIVER SRC=%u FINAL=%u HOPS=%u TTL=%u\r\n",
               h.src, h.final_dst, h.hop_count, h.ttl);
        return;
    }

    uint8_t next_hop = 0xFFu;
    if (t425_router_lookup(&router, h.final_dst, &next_hop)
        != T425_ROUTE_FORWARD) {
        dropped++;
        printf("DROP ROUTE FINAL=%u\r\n", h.final_dst);
        return;
    }

    if (!t425_forward_forwardable(&h)) {
        dropped++;
        printf("DROP TTL FINAL=%u\r\n", h.final_dst);
        return;
    }

    t425_forward_advance(&h, next_hop);
    uint8_t payload[T425_MAX_PAYLOAD];
    memcpy(payload, &h, sizeof(h));
    uint8_t app_len = f->len - (uint8_t)sizeof(h);
    memcpy(payload + sizeof(h), f->payload + sizeof(h), app_len);

    if (send_hop(next_hop, payload, f->len)) {
        forwarded++;
        printf("FORWARD SRC=%u FINAL=%u NEXT=%u HOPS=%u TTL=%u\r\n",
               h.src, h.final_dst, next_hop, h.hop_count, h.ttl);
    } else {
        dropped++;
    }
}

int main(void) {
    stdio_init_all();
    sleep_ms(1200);

    if (!t425_pio_phy_init(&phy, pio0, 0, 1,
                           T425_TX_PIN, T425_RX_PIN, T425_BAUD)) {
        printf("T425_MH V2.46 NODE=%u INIT=FAIL\r\n", T425_NODE_ID);
        while (1) sleep_ms(1000);
    }

    t425_pio_phy_reset_fifos(&phy);
    t425_rl_init(&rl, 50, 5);
    t425_parser_init(&parser);
    t425_router_init(&router, T425_NODE_ID);

    /* Same binary; only NODE_ID changes. */
    if (T425_NODE_ID == 0u) {
        t425_router_set_route(&router, 2u, 1u);
    } else if (T425_NODE_ID == 1u) {
        t425_router_set_route(&router, 0u, 0u);
        t425_router_set_route(&router, 2u, 2u);
    } else if (T425_NODE_ID == 2u) {
        t425_router_set_route(&router, 0u, 1u);
    }

    printf("T425_MH V2.46 NODE=%u BAUD=%lu\r\n",
           T425_NODE_ID, (unsigned long)T425_BAUD);

    uint32_t started = time_us_32() / 1000u;
    bool initiated = false;

    while (1) {
        uint8_t bytes[64];
        uint16_t n = t425_pio_phy_read(&phy, bytes, sizeof(bytes));

        for (uint16_t i = 0; i < n; ++i) {
            t425_frame_t f;
            if (!t425_parser_push(&parser, bytes[i], &f)) continue;
            if (f.dst != T425_NODE_ID) continue;

            rl.rx_frames++;

            if (f.type == T425_TYPE_ACK) {
                if (t425_rl_accept_ack(&rl, f.seq))
                    rl.next_tx_seq = (uint8_t)(f.seq + 1u);
            } else if (f.type == T425_TYPE_DATA) {
                process_data(&f);
            }
        }

        uint32_t now = time_us_32() / 1000u;

        /* P00 originates one packet every second, only when idle. */
        if (T425_NODE_ID == 0u &&
            !initiated && (now - started) >= 1000u) {
            make_initial_packet();
            initiated = true;
        }

        if (now - last_report >= 1000u) {
            last_report = now;
            printf("STAT NODE=%u RX=%lu TX=%lu ACKED=%lu RETX=%lu "
                   "TIMEOUT=%lu CRC=%lu DUP=%lu OOO=%lu FWD=%lu "
                   "DELIVER=%lu DROP=%lu\r\n",
                   T425_NODE_ID,
                   (unsigned long)rl.rx_frames,
                   (unsigned long)rl.tx_frames,
                   (unsigned long)rl.acked,
                   (unsigned long)rl.retransmits,
                   (unsigned long)rl.timeouts,
                   (unsigned long)parser.crc_errors,
                   (unsigned long)rl.duplicate_data,
                   (unsigned long)rl.out_of_order,
                   (unsigned long)forwarded,
                   (unsigned long)delivered,
                   (unsigned long)dropped);
        }

        sleep_ms(1);
    }
}
