#include <assert.h>
#include <limits.h>
#include "../shovel_rgds.c"
int main(void){
 uint32_t packet[1024]={0};mailbox=packet;state_checked=1;
 for(int level=0;level<=10;level++){
  packet[1]=level;float g=gain();
  assert(final_sample(99.f,g)==(int16_t)(32767*g));
  assert(final_sample(-99.f,g)==(int16_t)(-32767*g));
  assert(final_sample(NAN,g)==0&&final_sample(INFINITY,g)==0);
 }
 float in[4]={1,-1,.5f,-.5f},out[4]={0};int16_t dst[8];
 packet[1]=1;capture_pcm(NULL,in,out,2,2,2);assert(!memcmp(in,out,sizeof(in)));
 deliver_pcm(NULL,(uint8_t*)dst,sizeof(dst));assert(dst[0]==114&&dst[1]==-114&&dst[2]==57&&dst[3]==-57);for(int i=4;i<8;i++)assert(dst[i]==0);assert(!pcm_count);
 packet[1]=0;capture_pcm(NULL,in,out,2,2,2);deliver_pcm(NULL,(uint8_t*)dst,8);assert(!dst[0]&&!dst[1]&&!dst[2]&&!dst[3]&&!pcm_count);
 for(unsigned i=0;i<AUDIO_FRAMES+20;i++)capture_pcm(NULL,in,out,1,2,2);
 assert(pcm_count==AUDIO_FRAMES&&pcm_overflow==20);
 puts("PASS: all volume levels, startup level 1 amplitude, mute, nonfinite samples, saturation, stereo transfer, empty-ring silence and bounded overflow");return 0;
}
