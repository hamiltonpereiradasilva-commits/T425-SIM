#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_occam_chan.h"

typedef enum {
    T425_RDV_EMPTY=0,
    T425_RDV_WAIT_OUT,
    T425_RDV_WAIT_IN,
    T425_RDV_TRANSFER,
    T425_RDV_DONE
} t425_rdv_state_t;

typedef struct {
    t425_rdv_state_t state;
    int8_t out_proc;
    int8_t in_proc;
    t425_occam_message_t pending;
} t425_rendezvous_t;

void t425_rdv_init(t425_rendezvous_t *r);
bool t425_rdv_post_out(t425_rendezvous_t *r,uint8_t proc,
                       const uint8_t *data,uint8_t len);
bool t425_rdv_post_in(t425_rendezvous_t *r,uint8_t proc);
bool t425_rdv_transfer(t425_rendezvous_t *r,t425_occam_message_t *msg,
                       uint8_t *out_proc,uint8_t *in_proc);
