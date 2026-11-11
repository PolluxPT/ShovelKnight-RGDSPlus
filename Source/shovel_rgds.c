/* Standalone RGDS+ Shovel Knight integration. MIT, 2026 PolluxPT. */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
static void*sdl;static volatile uint32_t*mailbox;static int state_checked;
static void*sdlfn(const char*name){if(!sdl)sdl=dlopen("libSDL2-2.0.so.0",RTLD_NOW|RTLD_LOCAL);void*p=sdl?dlsym(sdl,name):NULL;if(!p){fprintf(stderr,"SHOVEL: required SDL function missing: %s\n",name);_exit(4);}return p;}
static void state_init(void){if(state_checked)return;state_checked=1;const char*p=getenv("SHOVEL_INPUT_STATE");if(p){int fd=open(p,O_RDONLY);if(fd>=0){void*m=mmap(NULL,4096,PROT_READ,MAP_SHARED,fd,0);close(fd);if(m!=MAP_FAILED)mailbox=m;}}}
static uint32_t physical(void){state_init();return mailbox?mailbox[0]:0;}
static float gain(void){state_init();unsigned level=mailbox?mailbox[1]:2;if(level>10)level=10;float x=level/10.f;return .35f*x*x;}
static int control_bit(int n){switch(n){case 0:return 16;case 1:return 17;case 2:return 18;case 3:return 19;case 4:return 26;case 6:return 27;case 9:return 24;case 10:return 25;case 11:return 22;case 12:return 23;case 13:return 20;case 14:return 21;default:return -1;}}
static int16_t merge_axis(int16_t value,int n){uint32_t b=physical();if(n==0&&(b&((1u<<20)|(1u<<21))))return ((b&(1u<<21))?32767:0)-((b&(1u<<20))?32767:0);if(n==1&&(b&((1u<<22)|(1u<<23))))return ((b&(1u<<23))?32767:0)-((b&(1u<<22))?32767:0);return value;}
/* Apply the handheld mailbox only to its built-in controller. External pads
 * retain native SDL state instead of mirroring the handheld in Showdown. */
static void*builtin_controller;
void*SDL_GameControllerOpen(int index){static void*(*real)(int);if(!real)real=sdlfn("SDL_GameControllerOpen");void*c=real(index);const char*(*name_for_index)(int)=sdlfn("SDL_JoystickNameForIndex");const char*name=name_for_index(index);if(c&&name&&(strstr(name,"ANBERNIC")||strstr(name,"anbernic")||strstr(name,"RGDS"))){builtin_controller=c;fprintf(stderr,"SHOVEL_INPUT: calibrated built-in controller index=%d name=%s\n",index,name);}return c;}
void SDL_GameControllerClose(void*c){static void(*real)(void*);if(!real)real=sdlfn("SDL_GameControllerClose");if(c==builtin_controller)builtin_controller=NULL;real(c);}
uint8_t SDL_GameControllerGetButton(void*c,int n){static uint8_t(*real)(void*,int);if(!real)real=sdlfn("SDL_GameControllerGetButton");int bit=control_bit(n);uint32_t b=physical();return mailbox&&builtin_controller&&c==builtin_controller&&bit>=0?(b>>bit)&1:real(c,n);}
int16_t SDL_GameControllerGetAxis(void*c,int n){static int16_t(*real)(void*,int);if(!real)real=sdlfn("SDL_GameControllerGetAxis");int16_t v=real(c,n);if(!builtin_controller||c!=builtin_controller)return v;if((n==4||n==5)&&mailbox)return (physical()&(1u<<(n==4?28:29)))?32767:0;return merge_axis(v,n);}
/* Attribute queries from the guest must describe the translated desktop GL
 * interface, while native SDL continues to own its real GLES2 context. */
static int parse_gl_version(const char*text,int*major,int*minor){
 if(!text)return 0;while(*text&&(*text<'0'||*text>'9'))text++;
 if(*text<'0'||*text>'9')return 0;char*end;long a=strtol(text,&end,10);if(end==text||*end!='.')return 0;text=end+1;long b=strtol(text,&end,10);if(end==text||a<1||a>9||b<0||b>99)return 0;*major=(int)a;*minor=(int)b;return 1;
}
static int translated_attribute(int attr,int*out){
 if((attr!=17&&attr!=18)||!out)return 0;
 void*(*proc)(const char*)=sdlfn("SDL_GL_GetProcAddress");const unsigned char*(*getstring)(unsigned)=proc("glGetString");int major=0,minor=0;
 const char*version=getstring?(const char*)getstring(0x1f02):NULL;
 if(!parse_gl_version(version,&major,&minor))return 0;
 *out=attr==17?major:minor;static unsigned reports;if(reports++<8)fprintf(stderr,"SHOVEL_GL: attribute %d = %d via translated GL_VERSION %s\n",attr,*out,version);return 1;
}
int SDL_GL_GetAttribute(int attr,int*out){static int(*real)(int,int*);if(!real)real=sdlfn("SDL_GL_GetAttribute");int rc=real(attr,out);if(translated_attribute(attr,out))return 0;return rc;}
int SDL_PollEvent(void*e){static int(*real)(void*);static int quit_sent;if(!real)real=sdlfn("SDL_PollEvent");const char*p=getenv("SHOVEL_EXIT_REQUEST");if(!quit_sent&&e&&p&&access(p,F_OK)==0){memset(e,0,56);*(uint32_t*)e=0x100;quit_sent=1;return 1;}return real(e);}
typedef struct{uint8_t data[16];} joystick_guid;
joystick_guid SDL_JoystickGetGUID(void*j){joystick_guid zero={{0}};int(*count)(void)=sdlfn("SDL_NumJoysticks");void*(*openj)(int)=sdlfn("SDL_JoystickOpen");void(*closej)(void*)=sdlfn("SDL_JoystickClose");int(*id)(void*)=sdlfn("SDL_JoystickInstanceID");joystick_guid(*guid)(int)=sdlfn("SDL_JoystickGetDeviceGUID");int wanted=id(j);for(int i=0;i<count();i++){void*other=openj(i);if(!other)continue;int match=id(other)==wanted;closej(other);if(match)return guid(i);}return zero;}
void SDL_GetJoystickGUIDInfo(joystick_guid g,uint16_t*vendor,uint16_t*product,uint16_t*version){uint16_t v=0,p=0,r=0;if(!g.data[6]&&!g.data[7]&&!g.data[10]&&!g.data[11]&&!g.data[14]&&!g.data[15]){v=g.data[4]|g.data[5]<<8;p=g.data[8]|g.data[9]<<8;r=g.data[12]|g.data[13]<<8;}if(vendor)*vendor=v;if(product)*product=p;if(version)*version=r;}
int SDL_IsJoystickNintendoSwitchPro(uint16_t v,uint16_t p){return v==0x057e&&(p==0x2009||p==0x200e);}
int SDL_IsJoystickPS4(uint16_t v,uint16_t p){return v==0x054c&&(p==0x05c4||p==0x09cc||p==0x0ba0);}
int SDL_IsJoystickXbox360(uint16_t v,uint16_t p){return v==0x045e&&(p==0x028e||p==0x028f||p==0x0719);}
int SDL_IsJoystickXboxOne(uint16_t v,uint16_t p){return v==0x045e&&(p==0x02d1||p==0x02dd||p==0x02e0||p==0x02ea);}
static int ensure_directory(const char*path){char*p=strdup(path);if(!p)return -1;for(char*q=p+1;;q++){if(*q=='/'||!*q){char c=*q;*q=0;if(*p&&mkdir(p,0700)&&errno!=EEXIST){free(p);return -1;}*q=c;if(!c)break;}}free(p);return 0;}
static void*owned_paths[32];
char*SDL_GetPrefPath(const char*org,const char*app){(void)org;(void)app;const char*p=getenv("SHOVEL_PREF_PATH");if(!p||!*p||ensure_directory(p))return NULL;size_t n=strlen(p);char*result=malloc(n+2);if(!result)return NULL;memcpy(result,p,n);if(!n||p[n-1]!='/')result[n++]='/';result[n]=0;for(int i=0;i<32;i++)if(!owned_paths[i]){owned_paths[i]=result;return result;}free(result);return NULL;}
void SDL_free(void*p){if(!p)return;for(int i=0;i<32;i++)if(owned_paths[i]==p){owned_paths[i]=NULL;free(p);return;}static void(*real)(void*);if(!real)real=sdlfn("SDL_free");real(p);}
static void*fm_next(const char*n){void*p=dlsym(RTLD_NEXT,n);if(!p){fprintf(stderr,"SHOVEL_AUDIO: missing FMOD interface %s\n",n);_exit(5);}return p;}
#include "audio.inc"
char*getenv(const char*name){static char*(*real)(const char*);if(!real)real=dlsym(RTLD_NEXT,"getenv");if(!real)return NULL;if(!strcmp(name,"HOME")){char*p=real("SHOVEL_SAVED_HOME");if(p&&*p)return p;}return real(name);}
