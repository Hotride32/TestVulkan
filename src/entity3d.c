#include <stdlib.h>
#include <string.h>


#include "simple_logger.h"
#include "entity3d.h"


typedef struct
{
    Entity *entity_list;
    Uint32 entity_max;
}EntityManager;

static EntityManager gf3d_entity_manager = {0};


void gf3d_entity_manager_close()
{
    if(gf3d_entity_manager.entity_list != NULL)
    {
        free(gf3d_entity_manager.entity_list);
    }
    memset(&gf3d_entity_manager,0,sizeof(EntityManager));
}


void gf3d_entity_manager_init(Uint32 entity_max)
{
    
    
    gf3d_entity_manager.entity_list = (Entity*)gfc_allocate_array(sizeof(Entity),entity_max);
    
    if(!gf3d_entity_manager.entity_list)
    {
        slog("failed to allocate entity list");
        return;
    }
    
    gf3d_entity_manager.entity_max = entity_max;
    atexit(gf3d_entity_manager_close);
}

Entity *gf3d_entity_new()
{
   // Entity *ent = NULL;
    int i;
    for (i = 0; i<gf3d_entity_manager.entity_max; i++)
    {
        if(gf3d_entity_manager.entity_list[i]._inuse)continue;
        
        memset(&gf3d_entity_manager.entity_list[i],0,sizeof(Entity));
        gf3d_entity_manager.entity_list[i]._inuse = 1;
        return &gf3d_entity_manager.entity_list[i];
    }
    slog("request for entity failed: all full up");
    return NULL;
}

Entity *gf3d_entity_get_list()
{
        return gf3d_entity_manager.entity_list;
    
}

Uint32 *gf3d_entity_get_max()
{
    
    
    return gf3d_entity_manager.entity_max;
}

void gf3d_entity_free(Entity *self)
{
    if (!self)
    {
        slog("self pointer is not valid");
        return;
    }
    self->_inuse = 0;
    gf3d_model_free(self->model);
    //gf3d_model_free(self->idle);
    //gf3d_model_free(self->walk);
    gf3d_body_clear(&self->body);
    if(self->data != NULL)
    {
        slog("warning: data not freed at entity free!");
    }
}

void gf3d_entity_draw(Entity *self)
{
  //  Vector4D color;
    if (!self)return;
    if (!self->_inuse)return;
 //   color = gf3d_color_to_vector4(self->color);
 //   gf3d_particle_emitter_draw(self->pe);
    if(self->stat == 0){
        gf3d_model_draw(
            self->model,
            &self->position,
            &self->rotation,
            self->bufferFrame,
            self->commandBuffer,
            self->modelMat);
        
    }
    else{
    gf3d_model_draw_anim(
        self->model,
        &self->position,
        &self->rotation,
        self->bufferFrame,
        self->commandBuffer,
        self->modelMat,
  //      
  //      &color,
        (Uint32) self->frameCount
        );
    }
    
    gf3d_body_draw(&self->body,self->position);
    
    if(self->draw != NULL)
    {
      self->draw(self);
    }
}

void gf3d_entity_draw_all(Uint32 bufferFrame, Uint32  frameCount, VkCommandBuffer commandBuffer)
{
   int i;
    for (i = 0; i < gf3d_entity_manager.entity_max;i++)
   {
      if (gf3d_entity_manager.entity_list[i]._inuse == 0)continue;
        gf3d_entity_manager.entity_list[i].commandBuffer = commandBuffer;
        //gf3d_entity_manager.entity_list[i].frameCount = frameCount;
        gf3d_entity_manager.entity_list[i].bufferFrame =bufferFrame;
        //gf3d_entity_manager.entity_list[i].key = key;
      gf3d_entity_draw(&gf3d_entity_manager.entity_list[i]);
    }
}


void gf3d_entity_update(Entity *self)
{
    if(!self)return;
    if(!self->_inuse)return;

    /*do collision testing*/
    vector3d_add(self->position,self->position,self->velocity);
    vector3d_add(self->velocity,self->velocity,self->acceleration);

    //slog("x"+self->position.x +"y"+self->position.y+"z"+self->positon.z);
    
    
    
    //gf3d_particle_emitter_update(self->pe);

    //gf3d_action_list_get_next_frame(self->al,&self->frameCount,self->action);
    /*
    gfc_matrix_identity(self->modelMat);
    self->bufferFrame = gf3d_vgraphics_render_begin();
       // gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),self->bufferFrame);
            self->commandBuffer = gf3d_command_rendering_begin(self->bufferFrame);
            */
               // self->frameCount +=0.025;
                //if (self->frameCount>=56){
                // self->frameCount =1;
                 
                //}
                
    //gf3d_body_draw(&self->body,self->position);
    
    if (self->update != NULL)
    {
        self->update(self);
   }
}



void gf3d_entity_think_all()
{
    int i;
    for (i = 0; i < gf3d_entity_manager.entity_max;i++)
    {
        if (gf3d_entity_manager.entity_list[i]._inuse == 0)continue;
        if (gf3d_entity_manager.entity_list[i].think != NULL)
        {
            gf3d_entity_manager.entity_list[i].think(&gf3d_entity_manager.entity_list[i]);
        }
    }
}

void gf3d_entity_update_all()
{
    int i;
    for (i = 0; i < gf3d_entity_manager.entity_max;i++)
    {
        if (gf3d_entity_manager.entity_list[i]._inuse == 0)continue;
        gf3d_entity_update(&gf3d_entity_manager.entity_list[i]);
    }
}


void gf3d_entity_pre_sync_body(Entity *self)
{
    if (!self)return;// nothin to do
    vector3d_copy(self->body.velocity,self->velocity);
    vector3d_copy(self->body.position,self->position);
}

void gf3d_entity_post_sync_body(Entity *self)
{
    if (!self)return;// nothin to do
    //slog("entity %li : %s old position(%f,%f,%f) => new position (%f,%f,%f)",self->id,self->name,self->position,self->body.position);
    vector3d_copy(self->position,self->body.position);
    vector3d_copy(self->velocity,self->body.velocity);
}


void gf3d_entity_pre_sync_all()
{
    int i;
    for (i = 0; i < gf3d_entity_manager.entity_max;i++)
    {
        if (gf3d_entity_manager.entity_list[i]._inuse == 0)continue;
        gf3d_entity_pre_sync_body(&gf3d_entity_manager.entity_list[i]);
    }
}

void gf3d_entity_post_sync_all()
{
    int i;
    for (i = 0; i < gf3d_entity_manager.entity_max;i++)
    {
        if (gf3d_entity_manager.entity_list[i]._inuse == 0)continue;
        gf3d_entity_post_sync_body(&gf3d_entity_manager.entity_list[i]);
    }
}



/*eol@eof*/
