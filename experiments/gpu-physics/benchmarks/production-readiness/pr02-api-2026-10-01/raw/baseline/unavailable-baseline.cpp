#include "box3d/box3d.h"
#include <cerrno>
#include <cstdio>
int main(){errno=0;auto p=b3CreateRecording(0);int e=errno;printf("recording_nonnull=%d errno=%d expected_ENOTSUP=%d\n",p!=nullptr,e,ENOTSUP);if(p)b3DestroyRecording(p);return p==nullptr&&e==ENOTSUP?0:1;}
