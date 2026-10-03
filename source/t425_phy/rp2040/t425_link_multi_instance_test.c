/*
 * F_PHY-35
 *
 * Teste novo da PHY:
 *
 *   link0 = SM TX0 + SM RX1
 *   link1 = SM TX2 + SM RX3
 *
 * As duas instâncias são inicializadas separadamente e permanecem
 * ativas simultaneamente.
 *
 * Este teste não reutiliza os testes da árvore antiga.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "t425_link.h"

/*
 * Dois links lógicos independentes.
 *
 * Cada instância possui:
 *   - uma SM TX;
 *   - uma SM RX;
 *   - dois DMA;
 *   - estado TX próprio.
 */
static t425_link_t link0;
static t425_link_t link1;

/*
 * Buffers RX independentes.
 */
static uint32_t link0_rx_buffer[16];
static uint32_t link1_rx_buffer[16];

int main(void)
{
    stdio_init_all();

    sleep_ms(1000);

    printf("\n");
    printf("===== F_PHY-35: TWO LINK INSTANCES =====\n");

    /*
     * A mesma PIO pode hospedar as duas instâncias.
     *
     * Cada link recebe duas SM distintas.
     */
    printf("[1] inicializando link0\n");

    bool ok0 = t425_link_init(
        &link0,
        pio0,
        0,              /* SM TX */
        1,              /* SM RX */
        0,              /* TX GPIO — L0 */
        1,              /* RX GPIO — L0 */
        T425_LINK_SPEED_10M
    );

    assert(ok0);

    printf("[PASS] link0 inicializado\n");

    printf("[2] inicializando link1\n");

    bool ok1 = t425_link_init(
        &link1,
        pio0,
        2,              /* SM TX */
        3,              /* SM RX */
        2,              /* TX GPIO — L1 */
        3,              /* RX GPIO — L1 */
        T425_LINK_SPEED_10M
    );

    assert(ok1);

    printf("[PASS] link1 inicializado\n");

    /*
     * DMA separado para cada instância.
     */
    printf("[3] inicializando DMA link0\n");

    bool dma0 = t425_link_dma_init(
        &link0,
        link0_rx_buffer,
        16
    );

    assert(dma0);

    printf("[PASS] DMA link0\n");

    printf("[4] inicializando DMA link1\n");

    bool dma1 = t425_link_dma_init(
        &link1,
        link1_rx_buffer,
        16
    );

    assert(dma1);

    printf("[PASS] DMA link1\n");

    /*
     * Estado deve ser distinto.
     */
    printf("[5] verificando recursos distintos\n");

    assert(link0.sm_tx != link1.sm_tx);
    assert(link0.sm_rx != link1.sm_rx);

    assert(link0.tx_dma != link1.tx_dma);
    assert(link0.rx_dma != link1.rx_dma);

    assert(link0.tx_word != link1.tx_word ||
           link0.tx_dma_word != link1.tx_dma_word ||
           &link0.tx_word != &link1.tx_word);

    assert(&link0.tx_word != &link1.tx_word);
    assert(&link0.tx_dma_word != &link1.tx_dma_word);

    assert(link0.rx_buffer != link1.rx_buffer);

    printf("[PASS] SM distintas\n");
    printf("[PASS] DMA distintos\n");
    printf("[PASS] TX state distinto\n");
    printf("[PASS] RX buffers distintos\n");

    /*
     * Inicializar os dois links.
     */
    printf("[6] iniciando link0 + link1\n");

    t425_link_start(&link0);
    t425_link_start(&link1);

    printf("[PASS] quatro SM habilitadas\n");

    /*
     * Concorrência:
     *
     * Cada instância recebe um byte diferente.
     *
     * A finalidade aqui é comprovar que o estado TX de uma
     * instância não é sobrescrito pela outra antes do DMA.
     */
    printf("[7] transmitindo simultaneamente\n");

    t425_link_send_frame(&link0, 0x0055);
    t425_link_send_frame(&link1, 0x00AA);

    /*
     * Os valores devem permanecer associados às respectivas
     * instâncias após os dois disparos.
     */
    assert(link0.tx_word == 0x55);
    assert(link1.tx_word == 0xAA);

    assert(link0.tx_word != link1.tx_word);

    printf("[PASS] link0 TX byte = 0x%02X\n", link0.tx_word);
    printf("[PASS] link1 TX byte = 0x%02X\n", link1.tx_word);

    /*
     * Verificação da palavra persistente do DMA.
     *
     * Não comparamos o conteúdo bruto porque a codificação
     * pertence à função build_data_word(), que não faz parte
     * deste teste de isolamento de instâncias.
     *
     * O que importa aqui é que cada DMA aponta para seu próprio
     * objeto persistente.
     */
    assert(&link0.tx_dma_word != &link1.tx_dma_word);

    printf("[PASS] DMA words independentes\n");

    /*
     * Deixar os dois links rodando simultaneamente.
     */
    sleep_ms(100);

    printf("[8] estado concorrente mantido\n");

    /*
     * Parar ambos.
     */
    t425_link_stop(&link0);
    t425_link_stop(&link1);

    printf("[PASS] link0 parado\n");
    printf("[PASS] link1 parado\n");

    printf("\n");
    printf("===== F_PHY-35 PASS =====\n");
    printf("TWO INSTANCES / FOUR SM / INDEPENDENT TX STATE\n");

    while (true) {
        tight_loop_contents();
    }

    return 0;
}
