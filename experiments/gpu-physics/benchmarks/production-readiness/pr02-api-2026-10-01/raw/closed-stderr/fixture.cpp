#include "box3d/box3d.h"
#include "native_api_status.h"
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>
int main(){assert(close(2)==0);gpu_b3_native_api_clear_error();assert(!b3SaveRecordingToFile(nullptr,nullptr));assert(errno==ENOTSUP);assert(strcmp(gpu_b3_native_api_last_error(),"b3SaveRecordingToFile")==0);puts("PASS immediate ENOTSUP and operation name with stderr closed");}
