#ifndef __SPAWN_H__
#define __SPAWN_H__

#include "entity3d.h"
#include "simple_json.h"

typedef struct
{
    const char *name;
    Entity *(*spawn)(Vector3D);
}Spawn;

void spawn_entity(const char *name,Vector3D position,Uint32 id,SJson *args);

#endif
