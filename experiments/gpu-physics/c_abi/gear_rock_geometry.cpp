#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
int main(){auto h=b3CreateRock(.3f);auto p=b3GetHullPoints(h);std::printf("[");for(int i=0;i<h->vertexCount;++i)std::printf("%s[%.9g,%.9g,%.9g]",i?",":"",p[i].x,p[i].y,p[i].z);std::puts("]");b3DestroyHull(h);}
