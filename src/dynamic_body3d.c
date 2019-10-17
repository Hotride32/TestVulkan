#include "dynamic_body3d.h"
#include "collision3d.h"
#include "simple_logger.h"
#include <stdlib.h>

Shape gf3d_dynamic_body_to_shape(DynamicBody *a)
{
    Shape aS = {0};
    if (!a)return aS;
    gf3d_shape_copy(&aS,*a->body->shape);
    gf3d_shape_move(&aS,a->position);
    return aS;
}

Vector3D gf3d_dynamic_body_bounce(DynamicBody *dba,Vector3D normal)
{
    Vector3D nv = {0};
    vector3d_reflect(&nv, normal,dba->velocity);
    return nv;
}

Collision *gf3d_dynamic_body_bounds_collision_check(DynamicBody *dba,Rect bounds,float timeStep)
{
    Collision *collision = NULL;
    Rect dbBounds;
    if (!dba)return NULL;
    if (!dba->body)
    {
        slog("cannot do collision check, body missing from DynamicBody");
        return NULL;
    }
    dbBounds = gf3d_shape_get_bounds(gf3d_dynamic_body_to_shape(dba));
    if ((dbBounds.x > bounds.x)&&(dbBounds.x + dbBounds.w < bounds.x + bounds.w)&&
        (dbBounds.y > bounds.y)&&(dbBounds.y + dbBounds.h < bounds.y + bounds.h))
    {
        // No collision with the level bounds
        return NULL;
    }
    collision = gf3d_collision_new();
    if (!collision)return NULL;
    collision->body = NULL;
    collision->timeStep = timeStep;
    //TODO: collision->pointOfContact;
    if (dbBounds.x <= bounds.x)collision->normal.x = 1;
    if (dbBounds.y <= bounds.y)collision->normal.y = 1;
    if (dbBounds.x + dbBounds.w >= bounds.x + bounds.w)collision->normal.x = -1;
    if (dbBounds.y + dbBounds.h >= bounds.y + bounds.h)collision->normal.y = -1;
    vector3d_normalize(&collision->normal);
    collision->shape = NULL;
    collision->bounds = 1;
    dba->blocked = 1;
    return collision;
}

Collision *gf3d_dynamic_body_shape_collision_check(DynamicBody *dba,Shape *shape,float timeStep)
{
    Collision *collision = NULL;
    if (!dba)return NULL;
    if ((!dba->body)||(!shape))
    {
        slog("cannot do collision check, body or shape shape missing");
        return NULL;
    }
    if (!gf3d_shape_overlap(gf3d_dynamic_body_to_shape(dba),*shape))
    {
        return NULL;
    }
    collision = gf3d_collision_new();
    if (!collision)return NULL;
    collision->body = NULL;
    collision->timeStep = timeStep;
    //TODO: collision->pointOfContact;
    collision->normal = gf3d_shape_get_normal_for_shape(*shape, gf3d_dynamic_body_to_shape(dba));
    collision->shape = shape;
    dba->blocked = 1;
    return collision;
}

Collision *gf3d_dynamic_body_collision_check(DynamicBody *dba,DynamicBody *dbb,float timeStep)
{
    Collision *collision = NULL;
    if ((!dba)||(!dbb))return NULL;
    if ((!dba->body)||(!dbb->body))
    {
        slog("cannot do collision check, body missing from one or more DynamicBody");
        return NULL;
    }
    if ((dba->body->team)&&(dbb->body->team)&&(dba->body->team == dbb->body->team))
    {
        return NULL;
    }
    if (!dba->body->cliplayer)
    {
        return NULL;
    }
    if (!gf3d_shape_overlap(gf3d_dynamic_body_to_shape(dba),gf3d_dynamic_body_to_shape(dbb)))
    {
        return NULL;
    }
    collision = gf3d_collision_new();
    if (!collision)return NULL;
    collision->body = dbb->body;
    collision->timeStep = timeStep;
    //TODO: collision->pointOfContact;
    collision->normal = gf3d_shape_get_normal_for_shape(gf3d_dynamic_body_to_shape(dbb),gf3d_dynamic_body_to_shape(dba));
    collision->shape = dbb->body->shape;
    if (dba->body->cliplayer & dbb->body->cliplayer)
    {
        dba->blocked = 1;
    }
    return collision;
}

DynamicBody *gf3d_dynamic_body_new()
{
    DynamicBody *db;
    db = (DynamicBody *)malloc(sizeof(DynamicBody));
    if (!db)
    {
        slog("failed to allocation data for a new dynamic body");
        return NULL;
    }
    memset(db,0,sizeof(DynamicBody));
    db->collisionList = gfc_list_new();
    return db;
}

void gf3d_dynamic_body_clear_collisions(DynamicBody *db)
{
    if (!db)return;
    gf3d_collision_list_clear(db->collisionList);
}

void gf3d_dynamic_body_free(DynamicBody *db)
{
    if (!db)return;
    //cleanup collionList
    gf3d_collision_list_free(db->collisionList);
    free(db);
}

void gf3d_dynamic_body_update(DynamicBody *db,float factor)
{
    if (!db)return;
    if (!db->body)return;
    vector3d_copy(db->body->position,db->position);
    vector3d_scale(db->body->velocity,db->velocity,factor);
}

void gf3d_dynamic_body_reset(DynamicBody *db,float factor)
{
    if (!db)return;
    db->blocked = 0;
    gf3d_dynamic_body_clear_collisions(db);
    vector3d_copy(db->position,db->body->position);
    vector3d_scale(db->velocity,db->body->velocity,factor);
    db->speed = vector3d_magnitude(db->velocity);
}

void gf3d_dynamic_body_resolve_overlap(DynamicBody *db,float backoff)
{
    int i,count;
    Collision *collision;
    Vector3D total = {0};
    if (!db)return;
    count = gfc_list_get_count(db->collisionList);
    for (i = 0; i < count; i++)
    {
        collision = (Collision*)gfc_list_get_nth(db->collisionList,i);
        if (!collision)continue;
        vector3d_add(total,total,collision->normal);
    }
    if (count)
    {
        vector3d_scale(total,total,(1.0/count)*backoff);
    }
    vector3d_add(db->body->position,db->body->position,total);
}

/*eol@eof*/
