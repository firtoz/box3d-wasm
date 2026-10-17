#include "box3d/box3d.h"
#include <cstdio>
#include <cstdlib>

extern "C" b3DistanceOutput ui17_previous_shape_distance(const b3DistanceInput*,b3SimplexCache*,b3Simplex*,int);
extern "C" b3DistanceOutput __wrap_b3ShapeDistance(const b3DistanceInput* input,b3SimplexCache* cache,b3Simplex* simplexes,int capacity){
    static int lastA=-1,lastB=-1;
    static unsigned sequence=0;
    const bool trace=std::getenv("GPU_PR10_CACHE_TRACE")&&simplexes&&capacity>0;
    const bool changed=trace&&(lastA!=input->proxyA.count||lastB!=input->proxyB.count);
    b3SimplexCache before={};
    bool valid=true;
    if(changed){
        lastA=input->proxyA.count;lastB=input->proxyB.count;before=*cache;
        valid=before.count<=4;
        for(int i=0;i<before.count&&i<4;++i)valid&=before.indexA[i]<input->proxyA.count&&before.indexB[i]<input->proxyB.count;
    }
    const b3DistanceOutput out=ui17_previous_shape_distance(input,cache,simplexes,capacity);
    if(changed)if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")){
        std::fprintf(f,"{\"api\":\"ui17_first_proxy_call\",\"sequence\":%u,\"A\":%d,\"B\":%d,\"cache_before\":{\"count\":%u,\"a\":[%u,%u,%u,%u],\"b\":[%u,%u,%u,%u]},\"valid_input_indices\":%s,\"cache_after_count\":%u,\"distance\":%.9g}\n",++sequence,input->proxyA.count,input->proxyB.count,(unsigned)before.count,(unsigned)before.indexA[0],(unsigned)before.indexA[1],(unsigned)before.indexA[2],(unsigned)before.indexA[3],(unsigned)before.indexB[0],(unsigned)before.indexB[1],(unsigned)before.indexB[2],(unsigned)before.indexB[3],valid?"true":"false",(unsigned)cache->count,out.distance);
        std::fclose(f);
    }
    return out;
}
