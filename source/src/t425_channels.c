#include "t425_channels.h"
#include <string.h>
void t425_channels_init(t425_channel_t*c){
 memset(c,0,sizeof(t425_channel_t)*T425_CHANNELS);
 for(int i=0;i<T425_CHANNELS;i++){c[i].state=CH_FREE;c[i].reader_pid=-1;c[i].writer_pid=-1;c[i].ack_writer_pid=-1;}
}
bool t425_chan_out(t425_channel_t*c,uint32_t pid,uint32_t value){
 if(c->state==CH_FREE&&c->ack_writer_pid==(int32_t)pid){
  c->ack_writer_pid=-1;return true;
 }
 if(c->state==CH_READY&&c->writer_pid==(int32_t)pid){
  c->state=CH_FREE;c->writer_pid=-1;c->ack_writer_pid=-1;return true;
 }
 if(c->state==CH_WAIT_IN){
  c->value=value;c->writer_pid=(int32_t)pid;c->state=CH_READY;
  return true;
 }
 c->value=value;c->writer_pid=(int32_t)pid;c->state=CH_WAIT_OUT;
 return false;
}
bool t425_chan_in(t425_channel_t*c,uint32_t pid,uint32_t*value){
 if(c->state==CH_READY&&c->reader_pid==(int32_t)pid){
  if(value)*value=c->value;
  c->state=CH_FREE;c->reader_pid=-1;c->writer_pid=-1;c->ack_writer_pid=-1;
  return true;
 }
 if(c->state==CH_WAIT_OUT){
  if(value)*value=c->value;
  c->ack_writer_pid=c->writer_pid;
  c->state=CH_READY;c->writer_pid=c->writer_pid;c->reader_pid=-1;
  return true;
 }
 c->reader_pid=(int32_t)pid;c->state=CH_WAIT_IN;
 return false;
}
