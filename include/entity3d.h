#ifndef _ENTITY3D_H_
#define _ENTITY3D_H_

#include "gf3d_model.h"
#include "body3d.h"
#include "shape3d.h"

typedef enum
{
    ES_Idle = 0,
    ES_Walking = 1,
    ES_Jumping = 2,
    ES_Seeking = 3,
    ES_Charging = 4,
    ES_Attacking = 5,
    ES_Cooldown = 6,
    ES_Leaving = 7,
    ES_Pain = 8,
    ES_Dying = 9,
    ES_Dead = 10
    
}EntityState;

typedef struct Entity_S
{
    TextLine name;
    Uint64 id;
    Uint8 _inuse;
    Model *model;
    Matrix4 modelMat;
    Uint32 bufferFrame;
    Uint32  frameCount;
    Uint32 stat;
    Shape shape;
    Body body;
    float Langle;
    float Rangle;
    VkCommandBuffer commandBuffer;
    Vector3D position;
    Vector3D rotation;
    Vector3D velocity;
    Vector3D acceleration;
    Vector3D scale;
    EntityState state;
    void (*think)(struct Entity_S *self);
    void (*update)(struct Entity_S *self);
    void (*touch)(struct Entity_S *self);
    void (*draw)(struct Entity_S *self);
    void (*activate)(struct Entity_S *self,struct Entity_S *activator);    /**<some entities can be activated by others, doors opened, levels, etc*/
    int  (*damage)(struct Entity_S *self,int amount, struct Entity_S *source);/**<when this entity takes damage*/
    void (*die)(struct Entity_S *self);     /**<when this entity dies*/
    void (*free)(struct Entity_S *self);
    float health;
    float healthmax;
    float cooldown;
    float jumpcool;
    float armor;
    float experience;
    float level;
    float otherStuff;
    void *data;
    
    
}Entity;



void gf3d_entity_manager_init(Uint32 entity_max); 


Entity *gf3d_entity_new();


void gf3d_entity_free(Entity *self);

void gf3d_entity_draw(Entity *self);

void gf3d_entity_draw_all(Uint32 bufferFrame, Uint32  frameCount, VkCommandBuffer commandBuffer);
//Uint8 key
void gf3d_entity_think_all();

void gf3d_entity_update_all();

void gf3d_entity_pre_sync_body(Entity *self);

void gf3d_entity_post_sync_body(Entity *self);

void gf3d_entity_pre_sync_all();

void gf3d_entity_post_sync_all();

#endif
