#pragma once
#include <stdint.h>
#include <stdbool.h>
#define T425_CHANNELS 16
typedef enum { CH_FREE=0, CH_WAIT_IN, CH_WAIT_OUT, CH_READY } t425_ch_state_t;
typedef struct {
    t425_ch_state_t state;
    uint32_t value;
    int32_t reader_pid;
    int32_t writer_pid;
    int32_t ack_writer_pid;
} t425_channel_t;
void t425_channels_init(t425_channel_t *ch);
bool t425_chan_out(t425_channel_t *c,uint32_t pid,uint32_t value);
bool t425_chan_in(t425_channel_t *c,uint32_t pid,uint32_t *value);
