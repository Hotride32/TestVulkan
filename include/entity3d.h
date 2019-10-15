#ifndef _ENTITY3D_H_
#define _ENTITY3D_H_

typedef enum
{
    ES_Idle = 0,
    ES_Dying = 1,
    ES_Dead = 2
    
    
}EntityState;

typedef struct Entity_S
{
    Uint8 _inuse;
    Model *model;
    Vector3 position;
    Vector3 rotation;
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 scale;
    EntityState state;
    void (think*)(struct Entity_S* self);
    void (update*)(struct Entity_S* self);
    void (touch*)(struct Entity_S* self);
    float health;
    float healthmax;
    float armor;
    float experience;
    float level;
    float otherStuff;
    void *data;
    
    
}



void gf3d_entity_manager_init(Uint32 entity_max); 


Entity *gf3d_entity_new();


void gf3d_entity_free(Entity *self);




#endif
