// Read-only access to upstream Rain's live Human definitions. Layout mirrors
// benchmarks.c (release RAIN_GRID_COUNT=10, RAIN_GROUP_SIZE=3), not engine state.
#include "human.h"
#include <assert.h>
typedef struct Group { Human humans[3]; } Group;
typedef struct RainData {
    Group* groups; b3MeshData* gridMesh; b3MeshData* torusMesh;
    int columnCount; int columnIndex;
} RainData;
extern RainData g_rainData;
typedef void Pr04HumanVisitor(const Human*,int,void*);
void pr04_visit_rain(Pr04HumanVisitor* visit,void* context) {
    for(int group=0;group<100;++group)for(int h=0;h<3;++h) {
        const Human* human=&g_rainData.groups[group].humans[h];
        if(human->isSpawned)visit(human,group*3+h,context);
    }
}
