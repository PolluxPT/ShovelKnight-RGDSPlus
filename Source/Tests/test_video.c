#define _GNU_SOURCE
#include <dlfcn.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
int main(int argc,char**argv){
 assert(argc==2);void*h=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);assert(h);
 int enabled=getenv("SHOVEL_NATIVE_GLES")&&*getenv("SHOVEL_NATIVE_GLES")== '1';
 int(*set)(int,int)=dlsym(h,"SDL_GL_SetAttribute");void*(*window)(const char*,int,int,int,int,uint32_t)=dlsym(h,"SDL_CreateWindow");void*(*context)(void*)=dlsym(h,"SDL_GL_CreateContext");void*(*proc)(const char*)=dlsym(h,"SDL_GL_GetProcAddress");void(*swap)(void*)=dlsym(h,"SDL_GL_SwapWindow");
 set(17,4);set(18,1);set(21,2);set(5,7);
 void*w=window("test",0,0,1024,768,9);assert(w==(void*)0x1234);assert(context(w)==(void*)0x5678);swap(w);
 int*a=dlsym(h,"attr");assert(a[5]==7);
 if(enabled){assert(a[17]==2&&a[18]==0&&a[21]==4);assert(*(uint32_t*)dlsym(h,"window_flags")==5);assert(*(int*)dlsym(h,"attachments")==1);assert(*(int*)dlsym(h,"flushes")==1);assert(*(int*)dlsym(h,"shows")==1);assert(proc("glGetString")== (void*)0x1111);assert(proc("unknown")== (void*)0x2222);}
 else {assert(a[17]==4&&a[18]==1&&a[21]==2);assert(*(uint32_t*)dlsym(h,"window_flags")==9);assert(!*(int*)dlsym(h,"attachments"));}
 assert(*(int*)dlsym(h,"swaps")==1);assert(dlsym(h,"missing_function")==NULL);puts(enabled?"PASS native GLES bridge":"PASS inactive native bridge passthrough");return 0;
}
