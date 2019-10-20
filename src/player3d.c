#include "player3d.h"
#include "simple_logger.h"
#include "gf3d_camera.h"
//#include "level.h"
#include "gfc_input.h"
//#include "entity_common.h"
//#include "particle_effects.h"
//#include "gui.h"

static Entity *_player = NULL;

void player_draw(Entity *self);
//void player_activate(Entity *self,Entity *activator);
void player_think(Entity *self);
void player_update(Entity *self);
int  player_touch(Entity *self,Entity *other);
//int  player_damage(Entity *self,int amount, Entity *source);
//void player_die(Entity *self);

typedef struct
{
    int baseSpeed;
    int maxSpeed;
    float baseAcceleration;
    float shields,shieldMax;
    float charge,chargeMax;
    float energy,energyMax;
    int capacitors,capacitorsMax;
    int attackType;
    
}PlayerData;


static PlayerData playerData = {
    3,50,3.6,
    0,100,
    0,100,
    0,100,
    0,0};

Entity *player_get()
{
    return _player;
}

void player_set_position(Vector3D position)
{
    if (!_player)
    {
        slog("no player loaded");
        return;
    }
    vector3d_copy(_player->position,position);
    vector3d_copy(_player->body.position,position);
}

void player_set_rotation(Vector3D rotation)
{
    
    Vector3D movedir = vector3d(0,0,-1);
    
    if (!_player)
    {
        slog("no player loaded");
        return;
    }
    //vector3d_angle_vectors(_player->rotation, &forward, &right, &up);
    
    //vector3d_angle_vectors(_player->rotation, NULL, NULL, &rotation);
    
    //vector3d_rotate_about_x(&_player->position, _player->rotation.z);
    
    
    //vector_angle(_player->rotation.x,_player->rotation.y)
    

    //vector3d_copy(_player->rotation, rotation);
    
    vector3d_angle_vectors(rotation, &movedir, NULL, NULL);
    
    //vector3d_set_angle_by_radians(&rotation, rotation.y);
    
    vector3d_copy(_player->rotation, rotation);
    
    slog("Rotation %f,%f,%f ", _player->rotation.x, _player->rotation.y, _player->rotation.z);
    
    //vector3d_clear(_player->rotation);
    
    //vector3d_copy(_player->rotation,rotation);
    //slog("Rotation %f,%f,%f ", _player->rotation.x, _player->rotation.y, _player->rotation.z);
    
    
    //vector3d_copy(_player->body.rotation,rotation);
}

Entity *player_spawn(Vector3D position)
{
    //,SJson *args
    if (_player != NULL)
    {
        vector3d_copy(_player->position,position);
        //level_add_entity(_player);
        return NULL;
    }
    return player_new(position);
}

Entity *player_new(Vector3D position)
{
    Entity *self;
    Vector3D rotation = vector3d(0,0,0);
    //Matrix4 modelMat2;
    self = gf3d_entity_new();
    if (!self)return NULL;
    gfc_line_cpy(self->name,"player");
    
    //gf2d_line_cpy(self->name,"player");
    //self->parent = NULL;
    
    
    self->shape = gf3d_shape_rect(-0.5, -0.5,-0.5, 0.5, 4,0.5);
    //self->shape = gf3d_shape_rect(-16, -16,-16, 30, 60,30);
    gf3d_body_set(
        &self->body,
        "player",
        1,
        1,
        //WORLD_LAYER,
        0,
        1,
        position,
        vector3d(0,0,0),
        10,
        1,
        0,
        &self->shape,
        self,
        NULL);

    
    
    //gf2d_actor_load(&self->actor,"actors/player.actor");
    //gf2d_actor_set_action(&self->actor,"idle");

   // self->sound[0] = gf2d_sound_load("sounds/jump_10.wav",1,-1);

   self->state = ES_Idle;
   
    
    vector3d_copy(self->position,position);
    
    
    
    vector3d_copy(self->scale,vector3d(1,1,1));
    //vector3d_set(self->scaleCenter,64,64);
    vector3d_set(self->rotation,0,0,0);
    
    
    
    //vector3d_angle_vectors(self->rotation,&rotation,&rotation,&rotation);
    
    //vector3d_set_angle_by_radians(&self->rotation,0);
    
   // vector2d_set(self->flip,0,0);
   // vector2d_set(self->facing,1,0);
    self->model = gf3d_model_load_animated("dun_idle",1,57);
    
    self->stat = 1;
    
    gfc_matrix_identity(self->modelMat);
    
    gfc_matrix_translate(
            self->modelMat,
           self->position
    );
    
    gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                114.75,
                vector3d(1,0,0));
    
    gf3d_body_draw(&self->body,self->position);
    
    //self->bufferFrame = gf3d_vgraphics_render_begin();
    //gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),self->bufferFrame);
    
//     test_spawn(self->position.x,0,0);
//     test_spawn(0,self->position.y,0);
//     test_spawn(0,0,self->position.z);
    
    //self->commandBuffer = gf3d_command_rendering_begin(self->bufferFrame);
    
   // self->frameCount = 1;
    self->think = player_think;
    self->draw = player_draw;
    self->update = player_update;
    //self->touch = player_touch;
    //self->damage = player_damage;
    //self->die = player_die;
    //self->free = gf3d_entity_free(self);
   // self->activate = player_activate;

    self->data = (void*)&playerData;
    
    self->health = self->healthmax = 100;
    _player = self;
   // level_add_entity(self);
    return self;
}

void player_draw(Entity *self)
{
    /*
    if (!_player)
    {
        slog("no player loaded");
        return;
    }
    gf3d_entity_draw(self);
    */
    
    self->frameCount +=0.025;
                if (self->frameCount>=56){
                 self->frameCount =1;
                 
                }
   
                
    
    //additional player drawings can go here
}
/*
void player_activation_check(Entity *self)
{
    entity_activate(self);
}
*/
void player_think(Entity *self)
{
    
    
    const Uint8 * keys;
    Matrix4 modelMat;
    float rotation = 0;
    /*
    gf3d_camera_look_at(
    vector3d(2,40,2),
    self->position,
    vector3d(0,0,1)
    );
    */
    
    keys = SDL_GetKeyboardState(NULL);
    float move;
    //self->angle = 0;
    
    gf3d_camera_set_view(self->modelMat);
    
    /*
    self->frameCount += (Uint32) 0.025;
                if (self->frameCount>= (Uint32) 56){
                 self->frameCount =(Uint32) 0;   
                }
    */
    /*
    Entity *pickup;
    pickup = entity_get_touching_pickup(self);
    if (pickup != NULL)
    {
        self->health += 25;
            slog("entity %li",self->health);
    }
    else
    {
       // slog("not touching player");
    }
    */
    switch (self->state)
    {
        case ES_Idle:
      
            
            
            if(keys == NULL){
        
            move = 0;
            //self->angle = 0;
        }
            if(keys[SDL_SCANCODE_D]){
          
           
           //move -=0.0000025;
         
           move = 0.005;
           
           if(keys[SDL_SCANCODE_W])
           {
           //gf3d_vgraphics_move_camera(self->modelMat, vector3d(-move,0,0));
           
           /*
           gfc_matrix_rotate(
                self->modelMat,
                modelMat,
                self->Rangle,
                vector3d(0,1,0));
           */
           
        
         /*
           vector3d_rotate_about_vector(
             &self->position,
             vector3d(0,0,1),
             self->position,
             -move
           );
           */
           
           //vector3d_normalize(&self->rotation);
           
          // vector3d_angle_vectors(self->position, &self->rotation,NULL, NULL);
           
           //player_set_position(vector3d((self->position.x-move),(self->position.y),(self->position.z)));
           
           vector3d_rotate_about_z(&self->position, -0.01);
           
           
           //player_set_position(vector3d((self->position.x),(self->position.y),(self->position.z)));
           
          // slog("Rotation %f,%f,%f ", _player->rotation.x, _player->rotation.y, _player->rotation.z);
           
           slog("Position %f,%f,%f ", _player->position.x, _player->position.y, _player->position.z);
           
           rotation -= 0.01;
           
           //vector3d_rotate_about_y(&self->position, move);
           
           
           /*
           self->Langle = 0;
           
           if(self->Rangle > -90){
                self->Rangle -= 45;
                
                gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                self->Rangle,
                vector3d(0,1,0));
                

           }
           else{
            gfc_matrix_rotate(
                    self->modelMat,
                    self->modelMat,
                    -self->Rangle + self->Rangle,
                    vector3d(0,1,0));
           }
           */
           
           
//            gfc_matrix_copy(modelMat,self->modelMat);
           
           
           gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                -0.01,
                vector3d(0,1,0));
           
           
           
           
           }    
           
           if(keys[SDL_SCANCODE_S])
           {
           
           vector3d_rotate_about_z(&self->position, 0.01);
           
           //vector3d_normalize(&self->position);
           
           
           slog("Position %f,%f,%f ", _player->position.x, _player->position.y, _player->position.z);
           
           rotation += 0.01;
         
           
           gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                0.01,
                vector3d(0,1,0));
           
           
           
           
           }
       }
       
       if(keys[SDL_SCANCODE_S]){
          
        
           //move -=0.00000025;
           
           
           move = 0.005;
           
           
           
           //vector3d_set_angle_by_radians(self->rotation,0.002);
           
           //player_set_position(vector3d(self->position.x,self->position.y+move,self->position.z));
           
           
           player_set_position(vector3d((self->position.x),self->position.y+move,self->position.z));
           
           //gf3d_vgraphics_move_camera(self->modelMat, vector3d(0,0,-move));
           
           gf3d_vgraphics_move_camera(self->modelMat, self->position);
           
           
           //gf3d_vgraphics_rotate_camera(move);
           
           /*
           gfc_matrix_make_translation(
            modelMat,
            vector3d(0,move,0)
            );
            */
           
           //if(move <= -0.25){
           //    move = -0.25;
           //}
           
           /* 
          gfc_matrix_make_translation(
            modelMat2,
            vector3d(0,0,move)
            );
        */
           /*
          self->position[0][3] += move;
          self->position[1][3] += move;
          self->position[2][3] += move;
          */
           
           
           /*
           
           self->Rangle = 0;
           
           if(self->Langle < 180){
                self->Langle += 180;
                
                gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                self->Langle/2,
                vector3d(0,1,0));
           }
           else{
            gfc_matrix_rotate(
                    self->modelMat,
                    self->modelMat,
                    self->Langle - self->Langle,
                    vector3d(0,1,0));
           }
           
           */
           //gfc_matrix_identity(modelmat);
           
           
           
           
           
           //Matrix4 modelmat = {{1,0,0,0},{0,-1,0,0},{0,0,1,0},{0,0,0,1}};
           
            //gfc_matrix_translate(
            //modelmat,
           // vector3d(0,-move,0)
           // );
            /*
            gfc_matrix_translate(
            modelmat,
            vector3d(self->position.x,self->position.y,self->position.z)
          );
            */
/*
           gfc_matrix_multiply(
               self->modelMat,
               modelmat,
               self->modelMat
               
           );
           */
           //self->modelMat[0][0] = modelmat[0][0];
          // self->modelMat[1][1] = -self->modelMat[1][1];
           //self->modelMat[2][2] = modelmat[2][2];
          // self->modelMat[3][3] = modelmat[3][3];
           
           /*
           gfc_matrix_translate(
            self->modelMat,
            vector3d(_player->position.x,_player->position.y,_player->position.z)
          );
           */
           
           slog("Position %f,%f,%f ", _player->position.x, _player->position.y, _player->position.z);
           
           //vector3d_copy(_player->position, vector3d(_player->position.x,_player->position.y +move,_player->position.z);
           
          
//           gfc_matrix_translate(
//             self->modelMat,
//             vector3d(move,move,0)
//           );
//           
          
          
          
          
          gfc_matrix_make_translation(
              self->modelMat,
              self->position);
          
          
          gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                114.75,
                vector3d(1,0,0));
          
          gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                rotation,
                vector3d(0,1,0));
          
          
          
          
          //gfc_matrix_identity(modelMat);
          /*
          gfc_matrix_copy(modelMat,self->modelMat);
          
          gfc_matrix_translate(
            self->modelMat,
            vector3d(0,move,0)
          );
          
          gfc_matrix_rotate(
                self->modelMat,
                modelMat,
                114.75,
                vector3d(1,0,0));
          
          gfc_matrix_rotate(
                modelMat,
                self->modelMat,
                180,
                vector3d(0,1,0));
          
          gfc_matrix_copy(self->modelMat,modelMat);
          */
          
       }
       if(keys[SDL_SCANCODE_W]){
          
           
        
           //move +=0.0000025;
           
           move = 0.005;
           
           //self->velocity.x += 0.5;
           
          // self->angle = 0;
           
            //player_set_position(vector3d(_player->position.x ,_player->position.y-move,_player->position.z));
           
           
            player_set_position(vector3d(self->position.x ,self->position.y-move,self->position.z));
        
           
           gf3d_vgraphics_move_camera(self->modelMat, self->position);
           
           
           //Z = vertical
           
          /* */ 
           //gf3d_vgraphics_move_camera(self->modelMat, vector3d(0,0,move));
           
          // gf3d_vgraphics_move_camera(vector3d(0,move,0));
           /*
           gfc_matrix_copy(modelMat,self->modelMat);
           
           gfc_matrix_translate(
               modelMat,
               vector3d(40,0,0)
           );
           
           gf3d_vgraphics_move_camera(modelMat,vector3d(0,-move,0));
           
          // gf3d_camera_set_position();
           
           //gf3d_vgraphics_move_camera(vector3d(move,0,0));
           */
           /*
            gfc_matrix_make_translation(
            vector3d(0,move,0)
            );
            */
            
           //if(move >= 0.25){
         //      move = 0.25;
           //}
          /* 
          gfc_matrix_make_translation(
            modelMat2,
            vector3d(0,0,move)
            );
        */
          
          
          slog("Position %f,%f,%f ", _player->position.x, _player->position.y, _player->position.z);
           
          
          /*
          self->Langle = 0;
          
          if(self->Rangle > -180){
                self->Rangle -=180;
                
                gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                self->Rangle,
                vector3d(0,1,0));
                

           }
           else{
            gfc_matrix_rotate(
                    self->modelMat,
                    self->modelMat,
                    -self->Rangle + self->Rangle,
                    vector3d(0,1,0));
           }
          
          */
          
          
//          gfc_matrix_translate(
//              self->modelMat,
//            vector3d(-move,-move,0)
//          );
          
         
          
          gfc_matrix_make_translation(
             self->modelMat,
             self->position);
          
          gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                rotation,
                vector3d(0,0,1));
          
          gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                114.75,
                vector3d(1,0,0));
          
//           gfc_matrix_rotate(
//                 self->modelMat,
//                 self->modelMat,
//                 rotation,
//                 vector3d(0,1,0));
//           
          
          
          
       }
       if(keys[SDL_SCANCODE_A]){
          
           //move +=0.0000025;
           
           move = 0.005;
           
           
           //player_set_position(vector3d(self->position.x+move,self->position.y,self->position.z));
           
           
           //player_set_position(vector3d(self->position.x+move,self->position.y,self->position.z));
           
           /* */
          // gf3d_vgraphics_move_camera(self->modelMat, vector3d(move,0,0));
           
           //vector3d_add(self->position,vector3d(move,0,0));
           
           /*
           self->Rangle = 0;
           
           if(self->Langle < 90){
                self->Langle += 45;
                
                gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                self->Langle/2,
                vector3d(0,1,0));
           }
           else{
            gfc_matrix_rotate(
                    self->modelMat,
                    self->modelMat,
                    self->Langle - self->Langle,
                    vector3d(0,1,0));
            
           }
           */
//             vector3d_rotate_about_vector(
//              &self->rotation,
//              vector3d(0,0,1),
//              self->rotation,
//              -move
//            );
//            
            //vector3d_normalize(&self->rotation);
           
           //vector3d_angle_vectors(self->position, NULL, NULL, &self->rotation);
           
           if(keys[SDL_SCANCODE_W])
           {
           vector3d_rotate_about_z(&self->position, 0.01);
           
           rotation += 0.01;
           
           
          //gf3d_vgraphics_move_camera(self->modelMat, vector3d(move,0,0));
           
           
            //player_set_position(vector3d((self->position.x+move),(self->position.y),(self->position.z)));
           
           //slog("Rotation %f,%f,%f ", _player->rotation.x, _player->rotation.y, _player->rotation.z);
           
           slog("Position %f,%f,%f ", _player->position.x, _player->position.y, _player->position.z);
           
           
            gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                0.01,
                vector3d(0,1,0));
           
           
           }
           
           
           if(keys[SDL_SCANCODE_S])
           {
           
           vector3d_rotate_about_z(&self->position, -0.01);
           
           
           slog("Position %f,%f,%f ", _player->position.x, _player->position.y, _player->position.z);
           
           rotation -= -0.01;
         
           
           gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                -0.01,
                vector3d(0,1,0));
           
           
           
           
           }
           
           
           
//            gfc_matrix_translate(
//               self->modelMat,
//             vector3d(move,0,0)
//          );
           
           //vector3d_angle_vectors(self->rotation,move,move,move);
           
           //gf3d_vgraphics_rotate_camera(0.5);
           
          
           
           /*
           gfc_matrix_rotate(
                modelMat,
                modelMat,
                0.002,
                vector3d(0,0,-1));
            */
            
           //important
            /*
           gfc_matrix_rotate(
                modelMat2,
                modelMat2,
                0.02,
                vector3d(0,1,0));
        */
        
       }
       if(keys[SDL_SCANCODE_T]){
           
           
           //player_set_position(vector3d(-move,0,0));
           
           //gf3d_vgraphics_rotate_camera(0.5);
           
           /*
           gfc_matrix_rotate(
                self->modelMat,
                modelMat,
                self->Rangle,
                vector3d(0,1,0));
           */
           
          self->model = gf3d_model_load("dino");
           self->stat = 0;
           /*
           self->Langle = 0;
           
           if(self->Rangle > -90){
                self->Rangle -= 45;
                
                gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                self->Rangle,
                vector3d(0,1,0));
                

           }
           else{
            gfc_matrix_rotate(
                    self->modelMat,
                    self->modelMat,
                    -self->Rangle + self->Rangle,
                    vector3d(0,1,0));
           }
           */
           
           /*
           gfc_matrix_translate(
            self->modelMat,
           vector3d(-move,0,0)
        );
        */
           if(keys[SDL_SCANCODE_T]){
           
           
//            player_set_position(vector3d(0,0,-move));
           
           //gf3d_vgraphics_rotate_camera(0.5);
           
           /*
           gfc_matrix_rotate(
                self->modelMat,
                modelMat,
                self->Rangle,
                vector3d(0,1,0));
           */
           
          
           
           /*
           self->Langle = 0;
           
           if(self->Rangle > -90){
                self->Rangle -= 45;
                
                gfc_matrix_rotate(
                self->modelMat,
                self->modelMat,
                self->Rangle,
                vector3d(0,1,0));
                

           }
           else{
            gfc_matrix_rotate(
                    self->modelMat,
                    self->modelMat,
                    -self->Rangle + self->Rangle,
                    vector3d(0,1,0));
           }
           */
           
           /*
           gfc_matrix_translate(
            self->modelMat,
           vector3d(0,0,move)
        );
          */ 
       }
       }
            /*
            if (gf2d_input_command_down("walkleft"))
            {
                self->flip.x = 1;
                self->facing.x = -1;
                if (!entity_wall_check(self,vector2d(-2,0)))
                {   
                    self->velocity.x -= 1.25;
                }
            }
            if (gf2d_input_command_down("walkright"))
            {
                self->flip.x = 0;
                self->facing.x = 1;
                if (!entity_wall_check(self,vector2d(2,0)))
                {
                    self->velocity.x += 1.25;
                }
            }
            if (((gf2d_input_command_pressed("jump"))&&(self->grounded))&&(self->jumpcool <= 0))
            {
                self->velocity.y -= 10;
                self->jumpcool = gf2d_actor_get_frames_remaining(&self->actor);
                gf2d_sound_play(self->sound[0],0,1,-1,-1);
                gf2d_actor_set_action(&self->actor,"jump");
            }
            if (gf2d_input_command_down("activate"))
            {
                slog("activating");
                self->cooldown  = 16;
                self->state = ES_Cooldown;
                player_activation_check(self);
            }
            if (gf2d_input_command_released("melee"))
            {
                gf2d_actor_set_action(&self->actor,"hack");
                self->cooldown = gf2d_actor_get_frames_remaining(&self->actor);
                self->state = ES_Attacking;
                self->attack = 1;
            }
            if (gf2d_input_command_released("shoot"))
            {
                gf2d_actor_set_action(&self->actor,"shoot");
                self->cooldown = gf2d_actor_get_frames_remaining(&self->actor);
                self->state = ES_Attacking;
                self->attack = 2;
            }
            */
            break;
        default:
            break;
    }
}
/*
void player_shoot(Entity *self)
{
    Entity *other;
    Vector2D start,end;
    Collision c;
    CollisionFilter filter = {0};
    filter.worldclip = 1;
    filter.cliplayer = MONSTER_LAYER;
    filter.ignore = &self->body;
    start.x = self->position.x + (self->facing.x * 25);
    start.y = self->position.y - 11;
    end.y = start.y;
    end.x =  self->position.x + (self->facing.x * 8000);// should be well outside of the level
    c = gf2d_collision_trace_space(level_get_space(), start, end ,filter);
    if (c.collided)
    {
        if ((c.body != NULL) && (c.body->data != NULL))
        {
            other = (Entity *)c.body->data;
            if (other->damage != NULL)
            {
                other->damage(other,5,self);
            }
        }
        particle_trail(start, c.pointOfContact,gf2d_color8(200,200,200,200));
    }
}

void player_melee(Entity *self)
{
    Shape s;
    int i,count;
    Entity *other;
    Collision *c;
    List *collisionList = NULL;
    s = gf2d_shape_rect(self->position.x + (self->flip.x * -48) + 16, self->position.y, 16, 32);
    collisionList = entity_get_clipped_entities(self,s, MONSTER_LAYER, 0);
    count = gf2d_list_get_count(collisionList);
    slog("hit %i targets",count);
    for (i = 0; i < count;i++)
    {
        c = (Collision*)gf2d_list_get_nth(collisionList,i);
        if (!c)continue;
        if (!c->body)continue;
        if (!c->body->data)continue;
        other = c->body->data;
        if (other->damage)other->damage(other,1,self);//TODO: make this based on weapon / player stats
    }
    gf2d_collision_list_free(collisionList);
}
*/
void player_update(Entity *self)
{
    
    //Vector3D camerpos = {0,0,0};
    //gf3d_camera_set_position(camerpos);
    /*
    gfc_matrix_translate(
      self->modelMat,  
        vector3d(0,-0.001,0)
    );
    */
    self->frameCount +=0.025;
                if (self->frameCount>=56){
                 self->frameCount =1;
                 
                }
    
    
    Vector3D camPosition = {0,0,0};
    if (!self)return;
    
    //if (self->maxHealth)gui_set_health(self->health/self->maxHealth);
    //camera_set_position(camPosition);
    //if (self->jumpcool > 0) self->jumpcool -= 0.2;
   // else self->jumpcool = 0;
    //world clipping

    
    // walk dampening
    if (self->velocity.x)
    {
        self->velocity.x *= 0.8;
        if (fabs(self->velocity.x) < 1)self->velocity.x = 0;
    }
    /*
    entity_world_snap(self);    // error correction for collision system
    entity_apply_gravity(self);
    switch (self->state)
    {
        case ES_Idle:
            break;
        case ES_Seeking:
            break;
        case ES_Attacking:
            if (gf2d_actor_get_frames_remaining(&self->actor) == 2)
            {
                if (self->attack == 1)
                {
                    player_melee(self);
                }
                else
                {
                    player_shoot(self);
                }
            }
        case ES_Jumping:
        case ES_Leaving:
        case ES_Charging:
        case ES_Pain:
        case ES_Cooldown:
            self->cooldown--;
            if (self->cooldown <= 0)
            {
                self->state = ES_Idle;
                gf2d_actor_set_action(&self->actor,"idle");
            }
            break;
        case ES_Walking:
        case ES_Dying:
            return;
        case ES_Dead:
            return;
    }*/
    
}

int player_touch(Entity *self,Entity *other)
{
    return 0;// player does not touch
}
/*
int player_damage(Entity *self,int amount, Entity *source)
{
    switch(self->state)
    {
        case ES_Dying:
        case ES_Dead:
        case ES_Leaving:
        case ES_Pain:
            slog("player cannot take damage now");
            return 0;
        default:
            break;
    }
    slog("player taking %i damage!",amount);
    self->health -= amount;
    //play pain sound, set pain state
    if (self->health <= 0)
    {
        self->health = 0;
        self->die(self);
    }
    return amount;//todo factor in shields
}

void player_die(Entity *self)
{
    
}

void player_activate(Entity *self,Entity *activator)
{
    
}
*/

/*eol@eof*/
