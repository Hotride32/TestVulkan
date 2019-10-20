

#include "body3d.h"
//#include "gf2d_draw.h"
#include "simple_logger.h"
#include <stdlib.h>

Uint8 gf3d_body_shape_collide(Body *a,Shape *s,Vector3D *poc, Vector3D *normal);

void gf3d_body_clear(Body *body)
{
    if (!body)return;
    memset(body,0,sizeof(Body));
}

void gf3d_body_push(Body *body,Vector3D direction,float force)
{
    if (!body)return;
    if (body->mass != 0)
    {
        force = force / body->mass;
    }
    vector3d_set_magnitude(&direction,force);
    vector3d_add(body->velocity,body->velocity,direction);
}

void gf3d_body_draw(Body *body,Vector3D offset)
{
    Vector4D color;
    Shape shape;
    Rect rect;
    Vector3D center;
    if (!body)return;
    vector4d_set(color,0,255,255,255);
    // draw center point
    vector3d_add(center,body->position,offset);
    //gf2d_draw_pixel(center,color);
        
    vector4d_set(color,255,0,255,255);
    gf3d_shape_copy(&rect,*body->shape);
    gf3d_shape_move(&rect,body->position);
   // gf3d_shape_draw(shape,gf2d_color_from_vector4(color),offset);
   // gf3d_rect_draw(rect);
}

void gf3d_body_set(
    Body       *body,
    char       *name,
    Uint8       worldclip,
    Uint32      cliplayer,
    Uint32      touchlayer,
    Uint32      team,
    Vector3D    position,
    Vector3D    velocity,
    float       mass,
    float       gravity,
    float       elasticity,
    Shape      *shape,
    void       *data,
    int         (*touch)(struct Body_S *self, List *collision))
{
    if (!body)return;
    body->cliplayer = cliplayer;
    body->touchlayer = touchlayer;
    body->team = team;
    body->worldclip = worldclip;
    vector3d_copy(body->position,position);
    vector3d_copy(body->velocity,velocity);
    body->mass = mass;
    body->gravity = gravity;
    body->elasticity = elasticity;
    body->shape = shape;
    body->data = data;
    body->touch = touch;
    gfc_word_cpy(body->name,name);
}

Shape gf3d_body_to_shape(Body *a)
{
    Shape aS = {0};
    if (!a)return aS;
    gf3d_shape_copy(&aS,*a->shape);
    gf3d_shape_move(&aS,a->position);
    return aS;
}

Uint8 gf3d_body_body_collide(Body *a,Body *b)
{
    
    if ((!a)||(!b))
    {
        slog("missing body in collision check");
        return 0;
    }
    return gf3d_shape_overlap(gf3d_body_to_shape(a),gf3d_body_to_shape(b));

    
}

/*eol@eof*/
