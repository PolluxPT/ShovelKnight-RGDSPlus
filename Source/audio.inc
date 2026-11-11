/* FMOD Ex 4.x keeps decoding/mixing; firmware SDL owns actual audio output.
 * NOSOUND here is a realtime FMOD mixer, not a mute: custom DSP transfers its
 * stereo PCM to a bounded ring drained by the native SDL audio callback. */
#include <pthread.h>
#include <math.h>
#define AUDIO_FRAMES 16384u
static float pcm_ring[AUDIO_FRAMES*2];
static unsigned pcm_read,pcm_write,pcm_count;
static pthread_mutex_t pcm_lock=PTHREAD_MUTEX_INITIALIZER;
static uint32_t audio_device;static void*audio_dsp;
static uint64_t pcm_received,pcm_nonzero,pcm_underrun,pcm_overflow;
static void(*close_audio)(uint32_t);
typedef struct {int freq;uint16_t format;uint8_t channels,silence;uint16_t samples,padding;uint32_t size;void(*callback)(void*,uint8_t*,int);void*userdata;} audio_spec;
typedef struct {
 char name[32];unsigned version;int channels;
 void *create,*release,*reset;
 int(*read)(void*,float*,float*,unsigned,int,int);
 void *setposition;int numparameters;void *paramdesc,*setparameter,*getparameter,*config;
 int configwidth,configheight;void*userdata;
} dsp_description;
static int capture_pcm(void*state,float*input,float*output,unsigned frames,int inch,int outch){
 (void)state;
 if(!input||inch<1||outch<1)return 0;
 if(output)for(unsigned i=0;i<frames;i++)for(int c=0;c<outch;c++)output[i*outch+c]=input[i*inch+(c<inch?c:0)];
 pthread_mutex_lock(&pcm_lock);
 for(unsigned i=0;i<frames;i++){
  if(pcm_count==AUDIO_FRAMES){pcm_read=(pcm_read+1)%AUDIO_FRAMES;pcm_count--;pcm_overflow++;}
  float l=input[i*inch],r=input[i*inch+(inch>1?1:0)];
  pcm_ring[pcm_write*2]=l;pcm_ring[pcm_write*2+1]=r;
  pcm_write=(pcm_write+1)%AUDIO_FRAMES;pcm_count++;pcm_received++;
  if(isfinite(l)&&isfinite(r)&&(l!=0||r!=0))pcm_nonzero++;
 }
 pthread_mutex_unlock(&pcm_lock);return 0;
}
static int16_t final_sample(float sample,float g){
 if(!isfinite(sample))return 0;
 if(sample>1)sample=1;else if(sample< -1)sample=-1;
 return (int16_t)(sample*g*32767.f);
}
static void deliver_pcm(void*unused,uint8_t*stream,int length){
 (void)unused;memset(stream,0,length);int16_t*out=(int16_t*)stream;float g=gain();
 pthread_mutex_lock(&pcm_lock);
 for(int i=0;i<length/4;i++){
  if(!pcm_count){pcm_underrun++;continue;}
  out[i*2]=final_sample(pcm_ring[pcm_read*2],g);
  out[i*2+1]=final_sample(pcm_ring[pcm_read*2+1],g);
  pcm_read=(pcm_read+1)%AUDIO_FRAMES;pcm_count--;
 }
 pthread_mutex_unlock(&pcm_lock);
}
static int start_audio(void*system){
 int(*init)(uint32_t)=sdlfn("SDL_InitSubSystem");
 uint32_t(*open_audio)(const char*,int,const audio_spec*,audio_spec*,int)=sdlfn("SDL_OpenAudioDevice");
 void(*pause_audio)(uint32_t,int)=sdlfn("SDL_PauseAudioDevice");close_audio=sdlfn("SDL_CloseAudioDevice");
 const char*(*error)(void)=sdlfn("SDL_GetError");
 int(*get_format)(void*,int*,int*,int*,int*,int*,int*)=fm_next("FMOD_System_GetSoftwareFormat");
 int rate=48000,format=0,channels=0,maxinput=0,resampler=0,bits=0;
 fprintf(stderr,"SHOVEL_AUDIO: querying mixer format\n");int rc=get_format(system,&rate,&format,&channels,&maxinput,&resampler,&bits);
 if(rc||channels!=2||rate<8000||rate>192000){fprintf(stderr,"SHOVEL_AUDIO: unsupported mixer format rc=%d rate=%d channels=%d\n",rc,rate,channels);return -1;}
 fprintf(stderr,"SHOVEL_AUDIO: opening SDL output rate=%d channels=%d\n",rate,channels);if(init(0x10)){fprintf(stderr,"SHOVEL_AUDIO: SDL init: %s\n",error());return -1;}
 audio_spec want={0},got={0};want.freq=rate;want.format=0x8010;want.channels=2;want.samples=2048;want.callback=deliver_pcm;
 audio_device=open_audio(NULL,0,&want,&got,0);
 if(!audio_device){fprintf(stderr,"SHOVEL_AUDIO: SDL open: %s\n",error());return -1;}
 if(got.freq!=rate||got.format!=0x8010||got.channels!=2){close_audio(audio_device);audio_device=0;return -1;}
 dsp_description desc={0};strcpy(desc.name,"RGDS PCM output");desc.version=1;desc.channels=0;desc.read=capture_pcm;
 int(*create)(void*,dsp_description*,void**)=fm_next("FMOD_System_CreateDSP");
 int(*add)(void*,void*,void**)=fm_next("FMOD_System_AddDSP");
 fprintf(stderr,"SHOVEL_AUDIO: creating DSP descriptor size=%zu\n",sizeof(desc));rc=create(system,&desc,&audio_dsp);fprintf(stderr,"SHOVEL_AUDIO: created DSP result=%d dsp=%p\n",rc,audio_dsp);if(!rc)rc=add(system,audio_dsp,NULL);
 if(rc){fprintf(stderr,"SHOVEL_AUDIO: PCM capture setup result=%d\n",rc);close_audio(audio_device);audio_device=0;return -1;}
 pause_audio(audio_device,0);
 fprintf(stderr,"SHOVEL_AUDIO: realtime FMOD PCM -> native SDL, %d Hz stereo S16; final gain %.5f\n",rate,gain());return 0;
}
int sk_event_init(void*,int,unsigned,void*,unsigned) __asm__("_ZN4FMOD11EventSystem4initEijPvj");
int sk_event_init(void*event,int channels,unsigned flags,void*extra,unsigned event_flags){
 static int(*real)(void*,int,unsigned,void*,unsigned);if(!real)real=fm_next("_ZN4FMOD11EventSystem4initEijPvj");
 int(*get_system)(void*,void**)=fm_next("FMOD_EventSystem_GetSystemObject");void*system=NULL;int rc=get_system(event,&system);if(rc||!system)return rc?rc:1;
 int(*output)(void*,int)=fm_next("FMOD_System_SetOutput");
 int(*format)(void*,int,int,int,int,int)=fm_next("FMOD_System_SetSoftwareFormat");
 int(*buffer)(void*,unsigned,int)=fm_next("FMOD_System_SetDSPBufferSize");
 if((rc=output(system,2))||(rc=format(system,48000,5,2,0,0))||(rc=buffer(system,1024,4))){fprintf(stderr,"SHOVEL_AUDIO: mixer configuration result=%d\n",rc);return rc;}
 fprintf(stderr,"SHOVEL_AUDIO: initializing real event mixer\n");rc=real(event,channels,flags,extra,event_flags);fprintf(stderr,"SHOVEL_AUDIO: mixer initialized result=%d\n",rc);
 if(!rc&&start_audio(system)){fprintf(stderr,"SHOVEL_AUDIO: stopped safely: native output unavailable\n");_exit(5);}
 fprintf(stderr,"SHOVEL_AUDIO: FMOD init result=%d\n",rc);return rc;
}
int sk_event_update(void*) __asm__("_ZN4FMOD11EventSystem6updateEv");
int sk_event_update(void*event){static int(*real)(void*);if(!real)real=fm_next("_ZN4FMOD11EventSystem6updateEv");int rc=real(event);static unsigned count;if(!(++count%600)){pthread_mutex_lock(&pcm_lock);fprintf(stderr,"SHOVEL_AUDIO: frames=%llu nonzero=%llu queued=%u underrun=%llu overflow=%llu gain=%.5f update=%d\n",(unsigned long long)pcm_received,(unsigned long long)pcm_nonzero,pcm_count,(unsigned long long)pcm_underrun,(unsigned long long)pcm_overflow,gain(),rc);pthread_mutex_unlock(&pcm_lock);}return rc;}
int sk_event_release(void*) __asm__("_ZN4FMOD11EventSystem7releaseEv");
int sk_event_release(void*event){static int(*real)(void*);if(!real)real=fm_next("_ZN4FMOD11EventSystem7releaseEv");if(audio_device){close_audio(audio_device);audio_device=0;}int rc=real(event);audio_dsp=NULL;return rc;}
int sk_event_start(void*) __asm__("_ZN4FMOD5Event5startEv");
int sk_event_start(void*event){static int(*real)(void*);if(!real)real=fm_next("_ZN4FMOD5Event5startEv");int rc=real(event);if(rc)fprintf(stderr,"SHOVEL_AUDIO: Event::start failure result=%d event=%p\n",rc,event);return rc;}
int sk_event_by_id(void*,unsigned,unsigned,void**) __asm__("_ZN4FMOD11EventSystem18getEventBySystemIDEjjPPNS_5EventE");
int sk_event_by_id(void*system,unsigned id,unsigned mode,void**event){static int(*real)(void*,unsigned,unsigned,void**);if(!real)real=fm_next("_ZN4FMOD11EventSystem18getEventBySystemIDEjjPPNS_5EventE");int rc=real(system,id,mode,event);if(rc)fprintf(stderr,"SHOVEL_AUDIO: event lookup failure result=%d id=%u mode=%x\n",rc,id,mode);return rc;}
