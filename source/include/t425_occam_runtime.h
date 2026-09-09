#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_occam_chan.h"

#define T425_OCCAM_MAX_PROCS 16
#define T425_OCCAM_MAX_CHANS 16
#define T425_OCCAM_QUEUE_DEPTH 8

typedef enum {
    T425_PROC_READY=0,
    T425_PROC_RUNNING,
    T425_PROC_BLOCKED_IN,
    T425_PROC_BLOCKED_OUT,
    T425_PROC_HALTED
} t425_proc_state_t;

typedef struct {
    uint8_t proc_id;
    t425_proc_state_t state;
    uint16_t pc;
    uint8_t priority;
} t425_occam_proc_t;

typedef struct {
    t425_occam_message_t q[T425_OCCAM_QUEUE_DEPTH];
    uint8_t head,tail,count;
    int8_t waiting_in;
    int8_t waiting_out;
} t425_occam_chan_runtime_t;

typedef struct {
    t425_occam_proc_t proc[T425_OCCAM_MAX_PROCS];
    t425_occam_chan_runtime_t chan[T425_OCCAM_MAX_CHANS];
    uint8_t proc_count,chan_count,current;
} t425_occam_runtime_t;

void t425_occam_runtime_init(t425_occam_runtime_t *r);
int t425_occam_proc_create(t425_occam_runtime_t *r,uint8_t priority);
bool t425_occam_chan_create(t425_occam_runtime_t *r,uint8_t *chan_id);
bool t425_occam_out(t425_occam_runtime_t *r,uint8_t proc_id,uint8_t chan_id,
                    const uint8_t *data,uint8_t len);
bool t425_occam_in(t425_occam_runtime_t *r,uint8_t proc_id,uint8_t chan_id,
                   t425_occam_message_t *msg);
void t425_occam_schedule(t425_occam_runtime_t *r);
