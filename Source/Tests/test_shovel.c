#include <assert.h>
#include <math.h>
#include "../shovel_rgds.c"
int main(void){uint32_t packet[1024]={0};mailbox=packet;state_checked=1;
 for(int level=0;level<=10;level++){packet[1]=level;float x=level/10.f;assert(fabsf(gain()-.35f*x*x)<1e-7f);}
 for(int mask=0;mask<16;mask++){packet[0]=(uint32_t)mask<<20;assert(merge_axis(1234,0)==((mask&3)?((mask&2)?32767:0)-((mask&1)?32767:0):1234));assert(merge_axis(4321,1)==((mask&12)?((mask&8)?32767:0)-((mask&4)?32767:0):4321));}
 assert(control_bit(0)==16&&control_bit(1)==17&&control_bit(2)==18&&control_bit(3)==19);assert(control_bit(9)==24&&control_bit(10)==25&&control_bit(4)==26&&control_bit(6)==27);
 joystick_guid g={{0x19,0,0xb6,0x55,1,0,0,0,1,0,0,0,0,1,0,0}};uint16_t v,p,r;SDL_GetJoystickGUIDInfo(g,&v,&p,&r);assert(v==1&&p==1&&r==256);SDL_GetJoystickGUIDInfo(g,NULL,NULL,NULL);assert(!SDL_IsJoystickPS4(v,p)&&!SDL_IsJoystickXbox360(v,p)&&!SDL_IsJoystickNintendoSwitchPro(v,p));
 const char*path=getenv("SHOVEL_PREF_PATH");assert(path);char*pref=SDL_GetPrefPath("ignored","ignored");assert(pref&&strstr(pref,path)==pref&&pref[strlen(pref)-1]=='/');SDL_free(pref);for(int i=0;i<32;i++)assert(!owned_paths[i]);
 puts("PASS: volume levels, direction combinations, release/native fallback, physical A/B/X/Y/L1/R1/SELECT/START, legacy GUID ABI and private preference allocation/free");return 0;}
