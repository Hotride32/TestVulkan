#include <stdlib.h>
#include <string.h>


#include "simple_logger.h"
#include "entity3d.h"
#include "simple_json.h"
#include "entity_common3d.h"

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
void gf3d_entity_damage(Entity *self,int damage)
{
    

    if (!self)return;
    if (!self->_inuse)return;
 
    self->health -= damage;
      if (self->health <= 0)
     {
         self->health = 0;
//        
         //gf3d_entity_free(self);
//         
     }
    
  
}

void gf3d_entity_attack(Entity *attacker,int damage, Uint32 target,int flip)
{
   int i;
     for (i = 0; i < gf3d_entity_manager.entity_max;i++)
    {
       if (gf3d_entity_manager.entity_list[i]._inuse == 0)continue;
       Entity *self = &gf3d_entity_manager.entity_list[i];
       Body * b = &self->body;
       Shape s =gf3d_body_to_shape(b);
//        if(gf3d_shape_overlap_poc(s,atta,NULL,NULL,flip) && gf3d_entity_manager.entity_list[i].body.cliplayer == target 
//            
//         ){
        if((vector3d_magnitude_between(attacker->position,self->position) < 10) && gf3d_entity_manager.entity_list[i].body.cliplayer == target 
            
         ){
             //slog("hit");
             gf3d_entity_damage(&gf3d_entity_manager.entity_list[i],damage);
             
         }
        
    }
}

Entity *gf3d_entity_iterate(Entity *start)
{
    Entity *p = NULL;
    if (!start)p = gf3d_entity_manager.entity_list;
    else 
    {
        p = start;
        p++;
    }
    for (;p != &gf3d_entity_manager.entity_list[gf3d_entity_manager.entity_max];p++)
    {
        if (p->_inuse)return p;
    }
    return NULL;
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

int sj_value_as_vector3d(SJson *json,Vector3D *output)
{
    const char *text = NULL;
    float numbers[4];
    int i,count;
    SJson *value;
    if (!json)return 0;
    if (sj_is_array(json))
    {
        count = sj_array_get_count(json);
        if (count < 3)return 0;
        if (count > 3)count = 3;
        for (i = 0; i < count;i++)
        {
            value = sj_array_get_nth(json,i);
            sj_get_float_value(value,&numbers[i]);
        }
        if (output)
        {
            output->x = numbers[0];
            output->y = numbers[1];
            output->z = numbers[2];
        }
        return 1;
    }
    if (sj_is_string(json))
    {
        text = sj_get_string_value(json);
        if(sscanf(text,"%f,%f,%f",&numbers[0],&numbers[1],&numbers[2]) != 3)
        {
            return 0;
        }
        if (output)
        {
            output->x = numbers[0];
            output->y = numbers[1];
            output->z = numbers[2];
        }
        return 1;
    }
    return 0;
}



void level_spawn_entities(SJson *spawnList)
{
    int i = 0, count = 0;
    SJson *item;
    Vector3D position;
    int id = 0;
    count  = sj_array_get_count(spawnList);
    for (i = 0; i < count; i++)
    {
        item = sj_array_get_nth(spawnList,i);
        if (!item)continue;
        sj_value_as_vector3d(sj_object_get_value(item,"position"),&position);
        if (!sj_get_integer_value(sj_object_get_value(item,"name"),&id))
        {
            id = 0;
        }
        spawn_entity(sj_get_string_value(sj_object_get_value(item,"name")),position,id,sj_object_get_value(item,"args"));
    }
}

void *level_info_load(char *filename)
{
    //LevelInfo *linfo = NULL;
    SJson *json,*world;
    SJson      *spawnList;
    if (!filename)return NULL;
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load level file %s",filename);
        return NULL;
    }
//     linfo = level_info_new();
//     if (!linfo)
//     {
//         return NULL;
//     }
    world = sj_object_get_value(json,"world");
    if (!world)
    {
        slog("missing world object in level file %s",filename);
        sj_free(json);
        //level_info_free(linfo);
        return NULL;
    }
   
    spawnList = sj_copy(sj_object_get_value(world,"spawnList"));
    
    
    sj_free(json);
    
    level_spawn_entities(spawnList);
    
    slog("loaded level info for %s",filename);
    return NULL;
}
/*
void level_save_entities(SJson *spawnList, Entity* self)
{
    int i = 0, count = 0;
    SJson *item;
    Vector3D position;
    int id = 0;
    count  = sj_array_get_count(spawnList);
    
    
    
     for (i = 0; i < count; i++)
     {
         item = sj_array_get_nth(spawnList,i);
         if (!item)continue;
         sj_value_as_vector3d(sj_object_get_value(item,"position"),&position);
         if (!sj_get_integer_value(sj_object_get_value(item,"name"),&id))
         {
             id = 0;
         }
         
         //spawn_entity(sj_get_string_value(sj_object_get_value(item,"name")),position,id,sj_object_get_value(item,"args"));
//     }
}

void *level_info_save(char *filename, Entity* self)
{
    //LevelInfo *linfo = NULL;
    SJson *json,*world;
    SJson      *spawnList;
    if (!filename)return NULL;
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load level file %s",filename);
        return NULL;
    }
//     linfo = level_info_new();
//     if (!linfo)
//     {
//         return NULL;
//     }
    world = sj_object_get_value(json,"world");
    if (!world)
    {
        slog("missing world object in level file %s",filename);
        sj_free(json);
        //level_info_free(linfo);
        return NULL;
    }
   
    spawnList = sj_copy(sj_object_get_value(world,"spawnList"));
    
   level_save_entities(spawnList); 
    
    
   
    sj_free(json);
    
   
    
    slog("saved level info for %s",filename);
    return NULL;
}*/




/*eol@eof*/
