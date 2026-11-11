#include <assert.h>
#include "../shovel_rgds.c"
int main(void){int major=0,minor=0,other=0;
 setenv("MOCK_GL_VERSION","2.1 gl4es wrapper",1);assert(SDL_GL_GetAttribute(17,&major)==0&&major==2);assert(SDL_GL_GetAttribute(18,&minor)==0&&minor==1);assert(major+minor/10.f>=2.1f);
 assert(SDL_GL_GetAttribute(21,&other)==0&&other==77);assert(SDL_GL_GetAttribute(999,&other)==-1&&other==77);assert(SDL_GL_GetAttribute(17,NULL)==-1);
 setenv("MOCK_GL_VERSION","OpenGL ES 2.0",1);assert(SDL_GL_GetAttribute(18,&minor)==0&&minor==0);assert(major+minor/10.f<2.1f);
 unsetenv("MOCK_GL_VERSION");assert(SDL_GL_GetAttribute(18,&minor)==0&&minor==0);
 setenv("MOCK_GL_VERSION","not a version",1);assert(SDL_GL_GetAttribute(17,&major)==0&&major==2);
 assert(parse_gl_version("4.6 Mesa",&major,&minor)&&major==4&&minor==6);assert(!parse_gl_version(NULL,&major,&minor));assert(!parse_gl_version("2.",&major,&minor));assert(!parse_gl_version("2x1",&major,&minor));
 puts("PASS: native GLES 2.0 / translated GL 2.1 mismatch, exact game assertion, unchanged unrelated attributes, no fabricated higher version, null/no-context/invalid-version fallback");return 0;}
