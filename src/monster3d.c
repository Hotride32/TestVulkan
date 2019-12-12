#include "monster3d.h"
//#include "level.h"
#include "player3d.h"
#include "simple_logger.h"
//#include "particle_effects.h"
#include "entity_common3d.h"

void monster_draw(Entity *self);
void monster_think(Entity *self);
void monster_update(Entity *self);
int  monster_touch(Entity *self,Entity *other);
int  monster_damage(Entity *self,int amount, Entity *source);
void monster_die(Entity *self);
int monster_player_sight_check(Entity *self);
void monster_think_hunting(Entity *self);
void monster_turn(Entity *self,int dir);
void monster_think_patroling(Entity *self);

Entity *monster_new(Vector3D position);

Entity *monster_spawn_at_player()
{
    //,SJson *args
    Entity *player = player_get();
    
    return monster_new(player->position);
    //,"actors/space_bug.actor"
}

Entity *monster_spawn(Vector3D position)
{
    //,SJson *args
    return monster_new(position);
    //,"actors/space_bug.actor"
}


Entity *monster_new(Vector3D position)
{
    
    //,char *actorFile
    
    Entity *self;
    self = gf3d_entity_new();
    if (!self)return NULL;
    
    gfc_line_cpy(self->name,"monster");
    //self->parent = NULL;
    
    self->shape = gf3d_shape_rect(-5, -5,-5 , 5, 5, 5);
    gf3d_body_set(
        &self->body,
        "monster",
        1,//world layer
        8,
        //MONSTER_LAYER
        0,
        2,
        position,
        vector3d(0,0,0),
        10,
        1,
        0,
        &self->shape,
        self,
        NULL);

   // gf3d_actor_load(&self->actor,actorFile);
    //gf3d_actor_set_action(&self->actor,"walk");
    
    //self->sound[0] = gf3d_sound_load("sounds/bug_attack1.wav",0.1,-1);
    //self->sound[1] = gf3d_sound_load("sounds/bug_pain1.wav",0.1,-1);

    
    vector3d_copy(self->position,position);
    
    vector3d_copy(self->scale,vector3d(1,1,1));
    //self->actor.al->scale
    //vector3d_set(self->scaleCenter,64,64);
    vector3d_set(self->rotation,64,64,0);
    //vector3d_set(self->flip,0,0);
    //vector3d_set(self->facing,-1,0);
    
    
    self->walk = gf3d_model_load_animated("geese_walk",1,41);
    self->idle = gf3d_model_load_animated("geese_attack",1,60);
    self->model = self->walk;
    
    self->stat = 1;
    
    
    self->maxFrame = 40;
    
    self->frameCount =1;
    
     //gf3d_model_load("dino");
//     
//     self->stat = 0;
//     
    gfc_matrix_identity(self->modelMat);
    
    gfc_matrix_translate(
            self->modelMat,
           self->position
    );
    
    
    gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
               90,
                vector3d(1,0,0));
    
    
    gf3d_body_draw(&self->body,self->position);
    
    self->think = monster_think_patroling;
    self->draw = monster_draw;
    self->update = monster_update;
    self->touch = monster_touch;
    self->damage = monster_damage;
    self->die = monster_die;
    //self->free = gf3d_entity_free(self);

    self->health = self->healthmax = 20;
    //level_add_entity(self);
    return self;
}

void monster_draw(Entity *self)
{
    
}

void monster_think_attacking(Entity *self)
{
    
    Entity *player = player_get();
    
   // if (gf3d_actor_get_frames_remaining(&self->actor) == 1)
    //{
        //slog("checking for hit");
        /*
        c = entity_block_hit(self,gf3d_rect(self->position.x + 32 + (self->flip.x * -64),self->position.y-8,16,32));
        if (c.collided)
        {
            other = c.body->data;
            slog("HIT %s",other->name);
            entity_damage(other,self,5,5);
        }
*/
    //}
    //if (gf3d_actor_get_frames_remaining(&self->actor) <= 0)
    //{
        //slog("back to search");
       if(self->maxFrame == 40){
            self->model = self->idle;
            self->maxFrame = 59;
            self->frameCount = 1;
       }
            //self->frameCount = 1;
            
    if (player->position.x > self->position.x)
    {
        monster_turn(self,1);
    }
    if (player->position.x < self->position.x)
    {
        monster_turn(self,-1);
    }
    
            
            
       if(self->frameCount >= 58){
           //slog("back to search");
        
        
        self->think = monster_think_hunting;
      
        
        
         }
    //}
}

void monster_attack(Entity *self)
{
    //slog("attacking player");
    //gf3d_actor_set_action(&self->actor,"attack1");
    //self->cooldown = gf3d_actor_get_frames_remaining(&self->actor);
    
    gf3d_entity_attack(self, 1, 1, 0);
    
    self->think = monster_think_attacking;
    //gf3d_sound_play(self->sound[1],0,1,-1,-1);
}

void monster_think_patroling(Entity *self)
{
    //slog("_patroling");
    
    Vector3D position = vector3d(self->position.x + 0.005, self->position.y, self->position.z);
    
    vector3d_copy(self->position,position);
    vector3d_copy(self->body.position,position);
    
   // slog("Monster position %f,%f,%f ", self->position.x, self->position.y, self->position.z);
    
    if (monster_player_sight_check(self))
    {
        self->think = monster_think_hunting;
        return;
    }/*
    if ((!entity_platform_end_check(self))||entity_wall_check(self, Vector3D (3 *self->facing.x,0)))
    {
        monster_turn(self,self->facing.x * -1);
    }
    self->velocity.x = 2 * self->facing.x;
    */
    //self->position.x = self->position.x *cos(45);
    //self->position = vector3d(self->position.x + 0.05, self->position.y, self->position.z);
    //Vector3D position = vector3d(self->position.x, self->position.y, self->position.z);
    
       // gfc_matrix_make_translation(
             // self->modelMat,
              //self->position);
        
        //self->body.position = vector3d(self->body.position.x, self->body.position.y, self->body.position.z);
    
    
}

void monster_turn(Entity *self,int dir)
{
    float rotation = 8;
    if (dir < 0)
    {
        
        gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                -rotation,
                vector3d(0,1,0));
        /*
        gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
               90,
                vector3d(1,0,0));*/
        
        //self->facing.x = -1;
        //self->flip.x = 0;
    }
    else if (dir > 0)
    {
        
        gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                rotation,
                vector3d(0,1,0));
        /*
        gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
               90,
                vector3d(1,0,0));*/
        
        //self->facing.x = 1;
        //self->flip.x = 1;
    }
}

void monster_think_hunting(Entity *self)
{
    Entity *player = player_get();
    
    //slog("Monster position %f,%f,%f ", self->position.x, self->position.y, self->position.z);
    
    //vector3d_normalize(&self->position);
    float keep = 0.0;
    if(self->maxFrame == 59){
        self->model = self->walk;
        self->maxFrame = 40;
        self->frameCount = 1;
    }
    
    if ((self->jumpcool) || (self->cooldown))return;
   // if (vector3d_magnitude_compare(vector3d(self->position.x - player->position.x,self->position.y - player->position.y, self->position.z - player->position.z),500) < 0)
    //{
      //  slog("moving towards player");
        
        Vector3D position = {0};
        
    float move = vector3d_magnitude_between(self->position,player->position);
        
    if (player->position.x > self->position.x)
    {
        monster_turn(self,1);
        position = vector3d(((self->position.x + player->position.x)/2.5), ((self->position.y + player->position.y)/2.5), self->position.z);
        
        //((self->position.y + player->position.y)/2.1)
        
//         position.x += 0.05;
       // keep +=1;
    }
    if (player->position.x < self->position.x)
    {
        monster_turn(self,-1);
        position = vector3d(((self->position.x + player->position.x)/2.5), ((self->position.y + player->position.y)/2.5), self->position.z);
        
        //keep = 0;
//         position.x -= 0.05;
    }
    
    
    
  /*  if(move > 10){
    position = vector3d(((self->position.x + player->position.x)/2.1), ((self->position.y + player->position.y)/2.1), 0); 
    
    } 
    if(move <10){
    
         position = vector3d((self->position.x + player->position.x)/2.1, (self->position.y + player->position.y)/2.1, 0);
    }
           */ 
    
     vector3d_copy(self->position,position);
    vector3d_copy(self->body.position,position);
    
    //slog("Monster position %f,%f,%f ", self->position.x, self->position.y, self->position.z);
       // return;
    //slog("%f", move);
    //}
    //monster loses sight of player
//     if (vector3d_magnitude_compare(vector3d(self->position.x - player->position.x,self->position.y - player->position.y, self->position.z - player->position.z),50) > 0)
//     {
    if(move >50){
        //slog("lost the player");
        self->think = monster_think_patroling;// idle think //_patroling
        //gf3d_actor_set_action(&self->actor,"walk");
        return;
    }
    //monster gets in range of player
    if (vector3d_magnitude_compare(vector3d(self->position.x - player->position.x,self->position.y - player->position.y, self->position.z - player->position.z),10) < 0)
    {
        monster_attack(self);
        return;
    }
    
    
    
    //slog("moving towards player");
    
    
    // jump to player
    //self->jumpcool = 20;
    //self->velocity.y = -10;
//    if (player->position.x < self->position.x)
//     {
//         monster_turn(self,1);
//         //Vector3D position = vector3d(self->position.x - player->position.x, self->position.y - player->position.y, self->position.z - player->position.z);
// 
//         
//         float move = vector3d_magnitude_between(self->position,player->position);
//         
// //         Vector3D position = vector3d(self->position.x - move, self->position.y - move, 0);
// //         
// //         
//         Vector3D position = vector3d((self->position.x - player->position.x)/2, (self->position.y + player->position.y)/2, 0);
//         
//      vector3d_copy(self->position,position);
//      vector3d_copy(self->body.position,position);
//      slog("Monster position %f,%f,%f ", self->position.x, self->position.y, self->position.z);
//      gfc_matrix_make_translation(
//          self->modelMat,
//          self->position);
//      
        
       // self->position = vector3d(self->position.x - player->position.x, 0, self->position.z - player->position.z);
//        gfc_matrix_make_translation(
//               self->modelMat,
//               self->position);
       // self->body.position = vector3d(self->body.position.x - player->body.position.x,0, self->body.position.z - player->body.position.z);
        
        //self->position.y - player->position.y
        
        //self->body.position.y - player->body.position.y
        
        //self->position = player->position;
        
        //self->velocity.x -= 15;
//     }
//     if (player->position.x > self->position.x)
//     {
        //monster_turn(self,-1);
        //Vector3D position = vector3d(self->position.x - player->position.x, self->position.y - player->position.y, self->position.z - player->position.z);
     
//         float move = vector3d_magnitude_between(self->position,player->position);
//         
//         Vector3D position = vector3d((self->position.x + player->position.x)/4, (self->position.y + player->position.y)/4, 0);
//         
//      vector3d_copy(self->position,position);
//     vector3d_copy(self->body.position,position);
    
//     gfc_matrix_make_translation(
//          self->modelMat,
//          self->position);
//     
    
   // gfc_matrix_translate(position);
    
    
        //slog("Monster position %f,%f,%f ", self->position.x, self->position.y, self->position.z);
        
    
        //self->position = vector3d(self->position.x - player->position.x, 0, self->position.z - player->position.z);
//         gfc_matrix_make_translation(
//               self->modelMat,
//               self->position);
       // self->body.position = vector3d(self->body.position.x - player->body.position.x, 0, self->body.position.z - player->body.position.z);
        
        
        //self->position = player->position;
        
        
        //self->velocity.x += 15;
    //}
    
//     slog("Monster position %f,%f,%f ", self->position.x, self->position.y, self->position.z);
}

void monster_think(Entity *self)
{
    if (monster_player_sight_check(self))
    {
        self->think = monster_think_hunting;
    }
}

void monster_update(Entity *self)
{
    
    Entity *player = player_get();
    if (self->jumpcool > 0) self->jumpcool -= 0.2;
    else self->jumpcool = 0;
    //world clipping
    if (self->cooldown > 0) self->cooldown--;
    if (self->cooldown < 0)self->cooldown = 0;
    //self->health -= 0.001;
    
    const Uint8 * keys;
    keys = SDL_GetKeyboardState(NULL);
    
    
     self->frameCount +=0.025;
                 if (self->frameCount >= self->maxFrame){
                  self->frameCount = 1;
                  
                 }
    /*
    gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                90,
                vector3d(1,0,0));
    */
    
    if(keys[SDL_SCANCODE_I]){
            self->health -=10;
    }
    
    if (self->health <= 0)
    {
        self->health = 0;
        //self->think = monster_die;
        
        //gf3d_model_free(self->walk);
        //gf3d_model_free(self->idle);
        gf3d_entity_free(self);
        //gf3d_actor_set_action(&self->actor,"death1");
    }
    
    /*
    // walk dampening
    if (self->velocity.x)
    {
        self->velocity.x *= 0.8;
        if (fabs(self->velocity.x) < 1)self->velocity.x = 0;
    }
    
    
    
    */
    
    //self->position.x += 0.0005;
    //self->position.y += 0.0005;
    
    
    
     gfc_matrix_make_translation(self->modelMat,self->position);
    
    gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                114.75,
                vector3d(1,0,0));
    
//     Vector3D position = vector3d(self->position.x - player->position.y, self->position.y - player->position.z, self->position.z - player->position.x);
//     
//     vector3d_copy(self->position,position);
//     vector3d_copy(self->body.position,position);
//    
//     gfc_matrix_make_translation(
//               self->modelMat,
//               self->position);
//           
//     
    //entity_apply_gravity(self);
    //entity_world_snap(self);    // error correction for collision system
    
    
    
}

int  monster_touch(Entity *self,Entity *other)
{
    slog("monster touch called");
    if (!other)return 0;
    if (gfc_line_cmp(other->name,"player") != 0)return 0;
    entity_damage(other,self,5,5);
    entity_push(self,other,5);
    return 0;
}

int  monster_damage(Entity *self,int amount, Entity *source)
{
    Vector3D dir = {0};
    slog("monster taking %i damage!",amount);
    self->health -= amount;
    //gf3d_sound_play(self->sound[1],0,0.1,-1,-1);
    vector3d_sub(dir,source->position,self->position);
    vector3d_normalize(&dir);
    vector3d_scale(dir,dir,3);
    //particle_spray(self->position, dir,gf3d_color8(240,0,0,255), 100);
    if (self->health <= 0)
    {
        self->health = 0;
        //self->think = monster_die;
        gf3d_entity_free(self);
        //gf3d_actor_set_action(&self->actor,"death1");
    }
    else
    {
       // gf3d_actor_set_action(&self->actor,"pain1");
        //self->cooldown = gf3d_actor_get_frames_remaining(&self->actor);
    }
    return amount;//todo factor in shields}
}

int monster_player_sight_check(Entity *self)
{
    Entity *player = player_get();
    if (!player)return 0;
    float move = vector3d_magnitude_between(self->position,player->position);
    
    
//     if (vector3d_magnitude_compare(vector3d (self->position.x - player->position.x,self->position.y - player->position.y, self->position.z - player->position.z),100) < 0)
    if(move < 50)
    {
        //gf3d_sound_play(self->sound[0],0,1,-1,-1);
        return 1;
    }
    return 0;
}

void monster_die(Entity *self)
{
    /*if (!gf3d_actor_get_frames_remaining(&self->actor))
    {
        self->dead = 1;
    }*/
}
/*eol@eof*/
