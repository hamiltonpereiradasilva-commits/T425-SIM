#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "t425_core.h"

static int failures;

static void expect_u32(const char *name, uint32_t got, uint32_t expected)
{
    if (got != expected) {
        fprintf(stderr,
                "FAIL: %s: got 0x%08x expected 0x%08x\n",
                name,
                got,
                expected);
        failures++;
    }
}

static void expect_true(const char *name, int condition)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        failures++;
    }
}

int main(void)
{
    t425_core_t core;
    t425_process_context_t *ctx;
    uint32_t wptr = 0x00001000u;
    uint32_t wdesc = wptr | 1u;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    expect_true("normal context initially invalid",
                core.process_contexts[0].valid == 0u);

    expect_true("interrupt context initially invalid",
                core.interrupt_contexts[0].valid == 0u);

    ctx = &core.process_contexts[0];

    ctx->valid = 1u;
    ctx->wptr = wptr;
    ctx->Wdesc = wdesc;
    ctx->Iptr = 0x00002000u;
    ctx->A = 0x11111111u;
    ctx->B = 0x22222222u;
    ctx->C = 0x33333333u;
    ctx->Ereg = 0x44444444u;
    ctx->Status = 0x55555555u;

    expect_u32("normal Wdesc", ctx->Wdesc, wdesc);
    expect_u32("normal Iptr", ctx->Iptr, 0x00002000u);
    expect_u32("normal A", ctx->A, 0x11111111u);
    expect_u32("normal B", ctx->B, 0x22222222u);
    expect_u32("normal C", ctx->C, 0x33333333u);
    expect_u32("normal Ereg", ctx->Ereg, 0x44444444u);
    expect_u32("normal Status", ctx->Status, 0x55555555u);

    core.interrupt_contexts[0].valid = 1u;
    core.interrupt_contexts[0].WdescIntSave = 0xAAAA0001u;
    core.interrupt_contexts[0].IptrIntSave = 0xAAAA0002u;
    core.interrupt_contexts[0].AregSave = 0xAAAA0003u;
    core.interrupt_contexts[0].BregSave = 0xAAAA0004u;
    core.interrupt_contexts[0].CregSave = 0xAAAA0005u;
    core.interrupt_contexts[0].StatusIntSave = 0xAAAA0006u;
    core.interrupt_contexts[0].EregIntSave = 0xAAAA0007u;

    expect_u32("interrupt WdescIntSave",
               core.interrupt_contexts[0].WdescIntSave,
               0xAAAA0001u);
    expect_u32("interrupt IptrIntSave",
               core.interrupt_contexts[0].IptrIntSave,
               0xAAAA0002u);
    expect_u32("interrupt AregSave",
               core.interrupt_contexts[0].AregSave,
               0xAAAA0003u);
    expect_u32("interrupt BregSave",
               core.interrupt_contexts[0].BregSave,
               0xAAAA0004u);
    expect_u32("interrupt CregSave",
               core.interrupt_contexts[0].CregSave,
               0xAAAA0005u);
    expect_u32("interrupt StatusIntSave",
               core.interrupt_contexts[0].StatusIntSave,
               0xAAAA0006u);
    expect_u32("interrupt EregIntSave",
               core.interrupt_contexts[0].EregIntSave,
               0xAAAA0007u);

    expect_u32("normal A remains independent",
               core.process_contexts[0].A,
               0x11111111u);
    expect_u32("normal Status remains independent",
               core.process_contexts[0].Status,
               0x55555555u);

    if (failures != 0) {
        fprintf(stderr,
                "F08.2-B context tests: FAIL (%d failures)\n",
                failures);
        return 1;
    }

    printf("F08.2-B context tests: PASS\n");
    return 0;
}
