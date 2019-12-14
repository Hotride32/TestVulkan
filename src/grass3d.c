#include "grass3d.h"
#include "player3d.h"
//#include "level.h"
#include "simple_logger.h"
//#include "entity_common.h"
//#include "gf2d_collision.h"

void grass_draw(Entity *self);
void grass_think(Entity *self);
void grass_update(Entity *self);
//int  grass_touch(Entity *self,Entity *other);
//int  grass_damage(Entity *self,int amount, Entity *source);
//void grass_die(Entity *self);
Entity *grass_new(Vector3D position);

Entity *grass_spawn(Vector3D position)
{
    //,SJson *args
    //if (!item){return NULL;}
    // TODO: Make an item system more robust
    
    return grass_new(position);
    /*
    if (strcmp(item,"essence") == 0)
    {
        return grass_new(position,item);
    }
    else if (strcmp(item,"crysalis") == 0)
    {
        return grass_new(position,item);
    }
    return NULL;
    */
}

Entity *grass_spawn_at_player()
{
    //,SJson *args
    Entity *player = player_get();
    slog("{ name : grass_start, position :[%f,%f,%f]} ", player->position.x, player->position.y, player->position.z);
    
    return grass_spawn(player->position);
    
}


Entity *grass_new(Vector3D position)
{
    Entity *self;
    self = gf3d_entity_new();
    if (!self)return NULL;
    
    gfc_line_cpy(self->name,"grass");
    
    //self->parent = NULL;
    /*
    self->shape = gf3d_shape_rect(-15, -15,-15 , 25, 30, 25);
    gf3d_body_set(
        &self->body,
        "item",
        1,
        2,
        //grass_LAYER,
        0,
        0,
        position,
        vector3d(0,0,0),
        10,
        1,
        0,
        &self->shape,
        self,
        NULL);*/
/*
    gf2d_actor_load(&self->actor,actorFile);
    gf2d_actor_set_action(&self->actor,"idle");
    
    self->sound[0] = gf2d_sound_load("sounds/essence.wav",1,-1);
    */
    self->state = ES_Idle;
   
    
    vector3d_copy(self->position,position);
    
    
    
    vector3d_copy(self->scale,vector3d(1,1,1));
    //vector3d_set(self->scaleCenter,64,64);
    vector3d_set(self->rotation,0,0,0);
   // vector2d_set(self->flip,0,0);
   // vector2d_set(self->facing,1,0);
    //self->model = gf3d_model_load_animated("grass",1,99);
    self->model = gf3d_model_load("grass");
    
    self->stat = 0;
    
    gfc_matrix_identity(self->modelMat);
    
    gfc_matrix_translate(
            self->modelMat,
           self->position
    );
    
    //self->bufferFrame = gf3d_vgraphics_render_begin();
    //gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),self->bufferFrame);
    
    
    //self->commandBuffer = gf3d_command_rendering_begin(self->bufferFrame);
    
    self->maxFrame = 99;
    
    self->frameCount = 1;
    
   // self->frameCount = 1;
    self->think = grass_think;
    self->draw = grass_draw;
    self->update = grass_update;
    //self->touch = player_touch;
    //self->damage = player_damage;
    //self->die = player_die;
    //self->free = gf3d_entity_free(self);
   // self->activate = player_activate;

    return self;
}

void grass_draw(Entity *self)
{
    
}

void grass_think(Entity *self)
{
    
    self->frameCount +=0.025;
                 if (self->frameCount>=self->maxFrame){
                  self->frameCount =1;
                  
                 }
    
    /*
    Entity *player;
    player = entity_get_touching_player(self);
    if (player != NULL)
    {
        //gf2d_sound_play(self->sound[0],0,1,-1,-1);
        //self->dead = 1;    
            slog("touching player");
    }
    else
    {
        slog("not touching player");
    }
    */
}

void grass_update(Entity *self)
{
    
    //entity_apply_gravity(self);

    
}
/*
int  grass_touch(Entity *self,Entity *other)
{
    return 0;
}

int  grass_damage(Entity *self,int amount, Entity *source)
{
    return 0;
}

void test_die(Entity *self)
{
    
}
*/
/*eol@eof*/
