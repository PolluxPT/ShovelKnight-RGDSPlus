#include <stdint.h>
#include <string.h>
int attr[32],attachments,flushes,swaps,shows;uint32_t window_flags;
int SDL_GL_SetAttribute(int a,int v){attr[a]=v;return 0;}
void*SDL_CreateWindow(const char*t,int x,int y,int w,int h,uint32_t f){window_flags=f;return (void*)0x1234;}
void*SDL_GL_CreateContext(void*w){return (void*)0x5678;}
void SDL_GL_SwapWindow(void*w){swaps++;}
void SDL_ShowWindow(void*w){shows++;}
void SDL_GL_GetDrawableSize(void*w,int*x,int*y){*x=1024;*y=768;}
void*SDL_GL_GetProcAddress(const char*n){return (void*)0x2222;}
void gl4es_rgds_attach_context(void){attachments++;}
void glFlush(void){flushes++;}
void*glXGetProcAddressARB(const char*n){return !strcmp(n,"glGetString")?(void*)0x1111:0;}
