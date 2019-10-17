#include "space3d.h"
//#include "gf3d_draw.h"
#include "simple_logger.h"
#include "dynamic_body3d.h"
#include <stdlib.h>


Uint8 gf3d_body_shape_collide(Body *a,Shape *s,Vector3D *poc, Vector3D *normal);

void gf3d_free_shapes(void *data,void *context)
{
    Shape *shape;
    if (!data)return;
    shape = (Shape*)data;
    free(shape);
}

void gf3d_free_dynamic_bodies(void *data,void *context)
{
    if (!data)return;
    gf3d_dynamic_body_free((DynamicBody*)data);
}

void gf3d_space_free(Space *space)
{
    if (!space)return;
    
    //static shapes ARE owned by the space, so are deleted when the space goes away
    gfc_list_foreach(space->staticShapes,gf3d_free_shapes,NULL);
    gfc_list_foreach(space->dynamicBodyList,gf3d_free_dynamic_bodies,NULL);
    gfc_list_delete(space->staticShapes);
    free(space);
}

Space *gf3d_space_new_full(
    int         precision,
    Rect        bounds,
    float       timeStep,
    Vector3D    gravity,
    float       dampening,
    float       slop)
{
    Space *space;
    space = gf3d_space_new();
    if (!space)return NULL;
    gf3d_rect_copy(space->bounds,bounds);
    vector3d_copy(space->gravity,gravity);
    space->timeStep = timeStep;
    space->precision = precision;
    space->dampening = dampening;
    space->slop = slop;
    return space;
}

Space *gf3d_space_new()
{
    Space *space;
    space = (Space *)malloc(sizeof(Space));
    if (!space)
    {
        slog("failed to allocate space for Space");
        return NULL;
    }
    memset(space,0,sizeof(Space));
    space->dynamicBodyList = gfc_list_new();
    space->staticShapes = gfc_list_new();
    return space;
}

void gf3d_space_add_static_shape(Space *space,Shape shape)
{
    Shape *newShape;
    if (!space)
    {
        slog("no space provided");
        return;
    }
    newShape = (Shape*)malloc(sizeof(shape));
    if (!newShape)
    {
        slog("failed to allocate new space for the shape");
        return;
    }
    memcpy(newShape,&shape,sizeof(Shape));
    space->staticShapes = gfc_list_append(space->staticShapes,(void *)newShape);
}

void gf3d_space_remove_body(Space *space,Body *body)
{
    int i,count;
    DynamicBody *db = NULL;
    if (!space)
    {
        slog("no space provided");
        return;
    }
    if (!body)
    {
        slog("no body provided");
        return;
    }
    if (space->dynamicBodyList)
    {
        count = gfc_list_get_count(space->dynamicBodyList);
        for (i = 0; i < count;i++)
        {
            db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
            if (!db)continue;
            if (db->body != body)continue;
            gf3d_dynamic_body_free(db);
            gfc_list_delete_nth(space->dynamicBodyList,i);
            break;
        }
    }
}

void gf3d_space_add_body(Space *space,Body *body)
{
    DynamicBody *db = NULL;
    if (!space)
    {
        slog("no space provided");
        return;
    }
    if (!body)
    {
        slog("no body provided");
        return;
    }
    db = gf3d_dynamic_body_new();
    if (!db)return;
    db->body = body;
    db->id = space->idpool++;
    space->dynamicBodyList = gfc_list_append(space->dynamicBodyList,(void *)db);
}

void gf3d_space_draw(Space *space,Vector3D offset)
{
   /*
    int i,count;
    SDL_Rect r;
    DynamicBody *db = NULL;
    if (!space)
    {
        slog("no space provided");
        return;
    }
    r = gf3d_rect_to_sdl_rect(space->bounds);
    vector3d_add(r,r,offset);    
    gf3d_draw_rect(r,vector4d(255,0,0,255));
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!db)continue;
        gf3d_body_draw(db->body,offset);
    }
    count = gfc_list_get_count(space->staticShapes);
    for (i = 0; i < count;i++)
    {
        gf3d_shape_draw(*(Shape *)gfc_list_get_nth(space->staticShapes,i),gf3d_color8(0,255,0,255),offset);
    }
    */
}

void gf3d_space_dynamic_bodies_world_clip(Space *space,DynamicBody *db, float t)
{
    int i,count;
    Shape *shape;
    Collision *collision;
    count = gfc_list_get_count(space->staticShapes);
    for (i = 0; i < count;i++)
    {
        shape = (Shape*)gfc_list_get_nth(space->staticShapes,i);
        if (!shape)continue;
        // check for layer compatibility
        collision = gf3d_dynamic_body_shape_collision_check(db,shape,t);
        if (collision == NULL)continue;
        db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
    }
    //check if the dynamic body is leaving the level bounds
    collision = gf3d_dynamic_body_bounds_collision_check(db,space->bounds,t);
    if (collision != NULL)
    {
        db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
    }
}

void gf3d_space_dynamic_bodies_step(Space *space,DynamicBody *db, float t)
{
    DynamicBody *other;
    Collision *collision;
    Vector3D oldPosition;
    Vector3D reflected,total;
    int normalCount;
    int i,count;
    if ((!space)||(!db))return;
    // save our place in case of collision
    vector3d_copy(oldPosition,db->position);
    vector3d_add(db->position,db->position,db->velocity);
    
    gf3d_dynamic_body_clear_collisions(db);    
    // check against dynamic bodies
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        other = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!other)continue;
        if (other == db)continue;   // skip checking outself
        // check for layer compatibility
        collision = gf3d_dynamic_body_collision_check(db,other,t);
        if (collision == NULL)continue;
        db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
    }

    if (db->body->worldclip)
    {
        gf3d_space_dynamic_bodies_world_clip(space,db, t);
    }
    if (db->blocked)
    {
        vector3d_copy(db->position,oldPosition);
        gf3d_dynamic_body_resolve_overlap(db,space->slop);
        if (db->body->elasticity > 0)
        {
            count = gfc_list_get_count(db->collisionList);
            vector3d_clear(total);
            normalCount = 0;
            for (i = 0; i < count; i++)
            {
                collision = (Collision*)gfc_list_get_nth(db->collisionList,i);
                if (!collision)continue;
                vector3d_add(db->position,db->position,collision->normal);
                reflected = gf3d_dynamic_body_bounce(db,collision->normal);
                if (vector3d_magnitude_squared(reflected) != 0)
                {
                    vector3d_add(total,total,reflected);
                    normalCount++;
                }
            }
            if (normalCount)
            {
// //                vector3d_scale(total,total,(1.0/normalCount)*space->slop);
//                db->velocity = total;
                vector3d_set_magnitude(&db->velocity,db->speed);
            }
        }
    }
}

void gf3d_space_step(Space *space,float t)
{
    DynamicBody *db = NULL;
    int i,count;
    if (!space)return;
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!db)continue;
        if (db->blocked)
        {
            continue;// no need to move something that has already collided
        }
        gf3d_space_dynamic_bodies_step(space,db, t);
    }
}

void gf3d_space_reset_bodies(Space *space)
{
    int i,count;
    if (!space)return;
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        gf3d_dynamic_body_reset((DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i),space->timeStep);
    }
}

void gf3d_space_update_bodies(Space *space,float loops)
{
    DynamicBody *db = NULL;
    int i,count;
    if (!space)return;
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!db)continue;
        gf3d_dynamic_body_update(db,loops);
    }
}

void gf3d_space_update(Space *space)
{
    float s;
    float loops = 0;
    if (!space)return;
    gf3d_space_fix_overlaps(space,8);
    gf3d_space_reset_bodies(space);
    // reset all body tracking
    for (s = 0; s <= 1; s += space->timeStep)
    {
        gf3d_space_step(space,s);
        loops = loops + 1;
    }
    gf3d_space_update_bodies(space,loops);
}

Uint8 gf3d_space_resolve_overlap(Space *space)
{
    DynamicBody *db = NULL;
    int i,count;
    int clipped = 0;
    if (!space)return 1;
    gf3d_space_reset_bodies(space);
    // for each dynamic body, get list of staic shapes that are clipped
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!db)continue;
        gf3d_space_dynamic_bodies_world_clip(space,db, 0);
        if (gfc_list_get_count(db->collisionList))
        {
            gf3d_dynamic_body_resolve_overlap(db,space->slop);
        }
    }
    return clipped;
}

void gf3d_space_fix_overlaps(Space *space,Uint8 tries)
{
    int i = 0;
    int done = 0;
    for (i = 0; (i < tries) & (done != 1);i++)
    {
        done = gf3d_space_resolve_overlap(space);
    }
}

/*eol@eof*/
