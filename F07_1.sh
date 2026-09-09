#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# T425-SIM — F07.1
# NOVO CORE / ISA FOUNDATION
#
# EXECUÇÃO EXCLUSIVAMENTE DENTRO DO CONTAINER
# ============================================================

PROJECT="$(pwd)"

if [ "$(basename "$PROJECT")" != "t425-canonical" ]; then
    echo "ERRO: execute a partir de /work/t425-canonical"
    echo "pwd atual: $PROJECT"
    exit 1
fi

CORE="$PROJECT/source/t425_core"
INC="$CORE/include"
SRC="$CORE/src"
CPU="$CORE/cpu"
MEM="$CORE/memory"
INS="$CORE/instruction"
PROC="$CORE/process"
SCHED="$CORE/scheduler"
TIMER="$CORE/timer"
CHAN="$CORE/channel"
FPU="$CORE/fpu"

TEST="$PROJECT/tests/t425_core"
ANALYSIS="$PROJECT/analysis"
OUT="$ANALYSIS/F07_saida.txt"

mkdir -p \
    "$INC" \
    "$SRC" \
    "$CPU" \
    "$MEM" \
    "$INS" \
    "$PROC" \
    "$SCHED" \
    "$TIMER" \
    "$CHAN" \
    "$FPU" \
    "$TEST" \
    "$ANALYSIS"

REPORT="$(mktemp)"

cleanup()
{
    rm -f "$REPORT"
}

trap cleanup EXIT

fail()
{
    echo
    echo "FALHA F07.1: $*"
    echo "CORE NÃO VALIDADO."
    cp "$REPORT" "$OUT"
    exit 1
}

exec > >(tee "$REPORT") 2>&1

echo "============================================================"
echo " T425-SIM F07.1 — NOVO CORE / ISA FOUNDATION"
echo "============================================================"
echo
echo "Projeto  : $PROJECT"
echo "CORE     : $CORE"
echo "Testes   : $TEST"
echo "Relatório: $OUT"
echo

echo "[1/8] CLOCK ARQUITETURAL"

cat > "$INC/t425_clock.h" <<'C_EOF'
#ifndef T425_CLOCK_H
#define T425_CLOCK_H

#include <stdint.h>

#define T425_CLOCK_HZ 20000000u
#define T425_CYCLE_NS 50u

typedef uint64_t t425_cycles_t;

#endif
C_EOF

echo "[2/8] CPU"

cat > "$INC/t425_cpu.h" <<'C_EOF'
#ifndef T425_CPU_H
#define T425_CPU_H

#include <stdint.h>

typedef struct
{
    uint32_t A;
    uint32_t B;
    uint32_t C;

    uint32_t Wptr;
    uint32_t Iptr;

    uint32_t Oreg;
    uint32_t Breg;
    uint32_t Creg;

    uint8_t error;
    uint8_t halt_on_error;
    uint8_t analyse;
    uint8_t halted;

    uint8_t j0_break;
    uint8_t timer_disabled_high;
    uint8_t timer_disabled_low;

    uint32_t devid;
} t425_cpu_state_t;

void t425_cpu_reset(t425_cpu_state_t *cpu);

#endif
C_EOF

cat > "$CPU/t425_cpu.c" <<'C_EOF'
#include "t425_cpu.h"

void t425_cpu_reset(t425_cpu_state_t *cpu)
{
    if (!cpu)
        return;

    *cpu = (t425_cpu_state_t){0};
}
C_EOF

echo "[3/8] MEMORY"

cat > "$INC/t425_memory.h" <<'C_EOF'
#ifndef T425_MEMORY_H
#define T425_MEMORY_H

#include <stdint.h>

#define T425_INTERNAL_RAM_BYTES 4096u

typedef struct
{
    uint8_t data[T425_INTERNAL_RAM_BYTES];
} t425_memory_t;

void t425_memory_reset(t425_memory_t *mem);

int t425_memory_read_word(
    const t425_memory_t *mem,
    uint32_t address,
    uint32_t *value);

int t425_memory_write_word(
    t425_memory_t *mem,
    uint32_t address,
    uint32_t value);

#endif
C_EOF

cat > "$MEM/t425_memory.c" <<'C_EOF'
#include "t425_memory.h"

static int valid_word(uint32_t address)
{
    return address < T425_INTERNAL_RAM_BYTES &&
           (address & 3u) == 0u;
}

void t425_memory_reset(t425_memory_t *mem)
{
    if (!mem)
        return;

    for (uint32_t i = 0; i < T425_INTERNAL_RAM_BYTES; ++i)
        mem->data[i] = 0;
}

int t425_memory_read_word(
    const t425_memory_t *mem,
    uint32_t address,
    uint32_t *value)
{
    if (!mem || !value || !valid_word(address))
        return -1;

    *value =
        ((uint32_t)mem->data[address + 0] << 0)  |
        ((uint32_t)mem->data[address + 1] << 8)  |
        ((uint32_t)mem->data[address + 2] << 16) |
        ((uint32_t)mem->data[address + 3] << 24);

    return 0;
}

int t425_memory_write_word(
    t425_memory_t *mem,
    uint32_t address,
    uint32_t value)
{
    if (!mem || !valid_word(address))
        return -1;

    mem->data[address + 0] = (uint8_t)(value >> 0);
    mem->data[address + 1] = (uint8_t)(value >> 8);
    mem->data[address + 2] = (uint8_t)(value >> 16);
    mem->data[address + 3] = (uint8_t)(value >> 24);

    return 0;
}
C_EOF

echo "[4/8] INSTRUCTION FOUNDATION"

cat > "$INC/t425_instruction.h" <<'C_EOF'
#ifndef T425_INSTRUCTION_H
#define T425_INSTRUCTION_H

#include <stdint.h>

typedef enum
{
    T425_OP_J     = 0x0,
    T425_OP_LDLP  = 0x1,
    T425_OP_PFIX  = 0x2,
    T425_OP_LDNL  = 0x3,
    T425_OP_LDC   = 0x4,
    T425_OP_LDNLP = 0x5,
    T425_OP_NFIX  = 0x6,
    T425_OP_LDL   = 0x7,
    T425_OP_ADC   = 0x8,
    T425_OP_CALL  = 0x9,
    T425_OP_CJ    = 0xA,
    T425_OP_AJW   = 0xB,
    T425_OP_EQC   = 0xC,
    T425_OP_STL   = 0xD,
    T425_OP_STNL  = 0xE,
    T425_OP_OPR   = 0xF
} t425_direct_opcode_t;

typedef struct
{
    uint8_t opcode;
    uint8_t operand_nibble;
} t425_instruction_t;

#endif
C_EOF

cat > "$INS/t425_instruction.c" <<'C_EOF'
#include "t425_instruction.h"

int t425_instruction_foundation_anchor(void)
{
    return (T425_OP_OPR == 0xF) ? 0 : -1;
}
C_EOF

echo "[5/8] CORE"

cat > "$INC/t425_core.h" <<'C_EOF'
#ifndef T425_CORE_H
#define T425_CORE_H

#include <stdint.h>

#include "t425_cpu.h"
#include "t425_memory.h"

typedef struct
{
    t425_cpu_state_t cpu;
    t425_memory_t memory;

    uint64_t cycles;
    uint64_t instructions;
} t425_core_t;

void t425_core_reset(t425_core_t *core);

#endif
C_EOF

cat > "$SRC/t425_core.c" <<'C_EOF'
#include "t425_core.h"

void t425_core_reset(t425_core_t *core)
{
    if (!core)
        return;

    t425_cpu_reset(&core->cpu);
    t425_memory_reset(&core->memory);

    core->cycles = 0;
    core->instructions = 0;
}
C_EOF

echo "[6/8] PROCESS / SCHEDULER / TIMER / CHANNEL / FPU"

cat > "$PROC/t425_process.h" <<'C_EOF'
#ifndef T425_CORE_PROCESS_H
#define T425_CORE_PROCESS_H

#include <stdint.h>

typedef uint32_t t425_wdesc_t;

#define T425_PRIORITY_HIGH 0u
#define T425_PRIORITY_LOW  1u

#define T425_WDESC_PRIORITY_MASK 1u
#define T425_WDESC_WPTR_MASK (~1u)

static inline uint32_t
t425_wdesc_wptr(t425_wdesc_t wdesc)
{
    return wdesc & T425_WDESC_WPTR_MASK;
}

static inline uint32_t
t425_wdesc_priority(t425_wdesc_t wdesc)
{
    return wdesc & T425_WDESC_PRIORITY_MASK;
}

#endif
C_EOF

cat > "$SCHED/t425_scheduler.h" <<'C_EOF'
#ifndef T425_CORE_SCHEDULER_H
#define T425_CORE_SCHEDULER_H

#include <stdint.h>

typedef struct
{
    uint32_t high_head;
    uint32_t high_tail;

    uint32_t low_head;
    uint32_t low_tail;

    uint8_t running;
} t425_scheduler_state_t;

void t425_scheduler_reset(
    t425_scheduler_state_t *scheduler);

#endif
C_EOF

cat > "$SCHED/t425_scheduler.c" <<'C_EOF'
#include "t425_scheduler.h"

void t425_scheduler_reset(
    t425_scheduler_state_t *scheduler)
{
    if (!scheduler)
        return;

    *scheduler = (t425_scheduler_state_t){0};
}
C_EOF

cat > "$TIMER/t425_timer.h" <<'C_EOF'
#ifndef T425_CORE_TIMER_H
#define T425_CORE_TIMER_H

#include <stdint.h>

typedef struct
{
    uint32_t high;
    uint32_t low;

    uint8_t enabled_high;
    uint8_t enabled_low;
} t425_timer_state_t;

void t425_timer_reset(
    t425_timer_state_t *timer);

#endif
C_EOF

cat > "$TIMER/t425_timer.c" <<'C_EOF'
#include "t425_timer.h"

void t425_timer_reset(
    t425_timer_state_t *timer)
{
    if (!timer)
        return;

    *timer = (t425_timer_state_t){0};

    timer->enabled_high = 1;
    timer->enabled_low = 1;
}
C_EOF

cat > "$CHAN/t425_channel.h" <<'C_EOF'
#ifndef T425_CORE_CHANNEL_H
#define T425_CORE_CHANNEL_H

#include <stdint.h>

typedef struct
{
    uint32_t address;

    uint8_t waiting_input;
    uint8_t waiting_output;
    uint8_t enabled;
} t425_channel_state_t;

#endif
C_EOF

cat > "$FPU/t425_fpu.h" <<'C_EOF'
#ifndef T425_CORE_FPU_H
#define T425_CORE_FPU_H

#include <stdint.h>

typedef struct
{
    uint8_t error;
    uint8_t rounding_mode;
} t425_fpu_state_t;

void t425_fpu_reset(
    t425_fpu_state_t *fpu);

#endif
C_EOF

cat > "$FPU/t425_fpu.c" <<'C_EOF'
#include "t425_fpu.h"

void t425_fpu_reset(
    t425_fpu_state_t *fpu)
{
    if (!fpu)
        return;

    *fpu = (t425_fpu_state_t){0};
}
C_EOF

echo "[7/8] ISA MANIFEST"

cat > "$CORE/t425_isa_manifest.csv" <<'CSV_EOF'
class,mnemonic,opcode,status
direct,J,0x0,VERIFIED
direct,LDLP,0x1,VERIFIED
direct,PFIX,0x2,VERIFIED
direct,LDNL,0x3,VERIFIED
direct,LDC,0x4,VERIFIED
direct,LDNLP,0x5,VERIFIED
direct,NFIX,0x6,VERIFIED
direct,LDL,0x7,VERIFIED
direct,ADC,0x8,VERIFIED
direct,CALL,0x9,VERIFIED
direct,CJ,0xA,VERIFIED
direct,AJW,0xB,VERIFIED
direct,EQC,0xC,VERIFIED
direct,STL,0xD,VERIFIED
direct,STNL,0xE,VERIFIED
direct,OPR,0xF,VERIFIED
opr,STARTP,0x0D,VERIFIED
opr,ENDP,0x03,VERIFIED
opr,RUNP,0x39,VERIFIED
opr,STOPP,0x15,VERIFIED
opr,LDPRI,0x1E,VERIFIED
opr,CSUB0,0x13,VERIFIED
CSV_EOF

echo "[8/8] TESTE DE FUNDAÇÃO"

cat > "$TEST/test_core_foundation.c" <<'C_EOF'
#include <assert.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_clock.h"
#include "t425_instruction.h"
#include "t425_process.h"

int main(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    assert(core.cpu.A == 0);
    assert(core.cpu.B == 0);
    assert(core.cpu.C == 0);

    assert(core.cpu.Wptr == 0);
    assert(core.cpu.Iptr == 0);

    assert(core.cycles == 0);
    assert(core.instructions == 0);

    assert(T425_CLOCK_HZ == 20000000u);
    assert(T425_CYCLE_NS == 50u);

    assert(T425_OP_J == 0x0);
    assert(T425_OP_LDLP == 0x1);
    assert(T425_OP_PFIX == 0x2);
    assert(T425_OP_OPR == 0xF);

    {
        t425_wdesc_t high =
            0x00001000u | T425_PRIORITY_HIGH;

        t425_wdesc_t low =
            0x00002000u | T425_PRIORITY_LOW;

        assert(t425_wdesc_wptr(high) == 0x00001000u);
        assert(t425_wdesc_priority(high) ==
               T425_PRIORITY_HIGH);

        assert(t425_wdesc_wptr(low) == 0x00002000u);
        assert(t425_wdesc_priority(low) ==
               T425_PRIORITY_LOW);
    }

    puts("F07.1 FOUNDATION TEST: PASS");

    return 0;
}
C_EOF

BUILD="$ANALYSIS/F07_build"

rm -rf "$BUILD"
mkdir -p "$BUILD"

cc \
    -std=c11 \
    -Wall \
    -Wextra \
    -Werror \
    -I"$INC" \
    -I"$PROC" \
    -I"$SCHED" \
    -I"$TIMER" \
    -I"$CHAN" \
    -I"$FPU" \
    "$CPU/t425_cpu.c" \
    "$MEM/t425_memory.c" \
    "$INS/t425_instruction.c" \
    "$SRC/t425_core.c" \
    "$SCHED/t425_scheduler.c" \
    "$TIMER/t425_timer.c" \
    "$FPU/t425_fpu.c" \
    "$TEST/test_core_foundation.c" \
    -o "$BUILD/test_core_foundation"

"$BUILD/test_core_foundation"

echo
echo "=== ISOLAMENTO DO NOVO CORE ==="

if find "$CORE" -type f -print |
   grep -Eq 't425_(vm|occam|pio|link|runtime|network|router)'; then

    fail "arquivo de camada proibida encontrado dentro do novo CORE"
fi

echo
echo "=== ARQUIVOS CRIADOS ==="

find "$CORE" -type f -print | sort

echo
echo "============================================================"
echo " F07.1 FOUNDATION TEST: PASS"
echo "============================================================"
echo
echo "Novo CORE criado."
echo "CORE legado não foi modificado."
echo "PIO não integrado."
echo "OCCAM não integrado."
echo "Helios não integrado."
echo "Nenhum commit executado."

cp "$REPORT" "$OUT"

echo
echo "Relatório: $OUT"
