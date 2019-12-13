#include "spawn3d.h"
//#include "level.h"
#include "player3d.h"
#include "monster3d.h"
#include "pickup3d.h"
//#include "door.h"
//#include "breakable.h"
#include "simple_logger.h"

static Spawn spawnlist[] = 
{
    {
        "player_start",
        player_spawn
    },
    {
        "monster_start",
        monster_spawn
    },
    {
        "pickup_start",
        pickup_spawn
    },
 /*   {
        "breakable",
        breakable_spawn,
    },
    {
        "door",
        door_spawn
    },*/
    {0}
};

void spawn_entity(const char *name,Vector3D position,Uint32 id,SJson *args)
{
    Spawn *spawn;
    Entity *ent;
    if (!name)
    {
        slog("no spawn name provided");
        return;
    }
    for (spawn = spawnlist; spawn->name != 0; spawn++)
    {
        if (strcmp(spawn->name,name)==0)
        {
            if (spawn->spawn)
            {
                ent = spawn->spawn(position);
                if (ent)
                {
                    ent->id = id;
                }
                return;
            }
        }
    }
    slog("failed to spawn entity %s",name);
}


/*eol@eof*/
