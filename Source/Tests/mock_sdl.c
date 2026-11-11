#include <stdlib.h>
#include <string.h>
int SDL_GL_GetAttribute(int attr,int*out){if(!out)return -1;*out=attr==17?2:attr==18?0:77;return attr==17||attr==18||attr==21?0:-1;}
static const unsigned char*version(unsigned n){return n==0x1f02?(const unsigned char*)getenv("MOCK_GL_VERSION"):NULL;}
void*SDL_GL_GetProcAddress(const char*name){return !strcmp(name,"glGetString")?(void*)version:NULL;}
