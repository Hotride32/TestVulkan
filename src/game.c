#include <SDL.h>            

#include "simple_logger.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "entity3d.h"

int main(int argc,char *argv[])
{
    int done = 0;
    float move = 0;
    float pos;
    int a;
    Uint8 validate = 1;
    float frame = 1;
    const Uint8 * keys;
    Uint32 bufferFrame = 0;
    VkCommandBuffer commandBuffer;
    Model *model = NULL;
    Matrix4 modelMat;
   // Model *model2 = NULL;
    Matrix4 modelMat2;
    Entity *player = NULL;
    
    //vector3d(0,0,0);
    
    //vector3d(1,1,1);
    
    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"-disable_validate") == 0)
        {
            validate = 0;
        }
    }
    
    init_logger("gf3d.log");    
    slog("gf3d begin");
    gf3d_vgraphics_init(
        "gf3d",                 //program name
        1200,                   //screen width
        700,                    //screen height
        vector4d(0.51,0.75,1,1),//background color
        0,                      //fullscreen
        validate                //validation
    );
    
    
    
   gf3d_entity_manager_init(2028);
    
   //player->model = gf3d_model_load("dino");
   //player->bufferFrame
   
   player_spawn(vector3d(1,1,0));
   
   
   
   
   //pickup_spawn(vector3d(10,0,0));
  // pickup_spawn(vector3d(0,0,10));
   //pickup_spawn(vector3d(-10,0,0));
   pickup_spawn(vector3d(10,0,0));
   //pickup_spawn(vector3d(0,0,-10));
   
   //player_spawn(vector3d(10,0,0));
    // main game loop
    slog("gf3d main loop begin");
   // model = gf3d_model_load("dino");
    //model = gf3d_model_load_animated("dun_idle",1,57);
    //gfc_matrix_identity(modelMat);
    //model2 = gf3d_model_load("dino");
    //model2 = gf3d_model_load("dito");
    //gfc_matrix_identity(modelMat2);
    
    /*
     //player = gf3d_entity_new;
    player->model = model;
    //player->modelMat = modelMat2;
    //gfc_matrix_identity(player->modelMat);
    
    vector3d_copy(player->position,vector3d(0,0,0));
    
    vector3d_copy(player->scale,vector3d(1,1,1));
   // vector3d_set(player->scaleCenter,64,64,);
    vector3d_set(player->rotation,0,0,0);
    
   // self->think = player_think;
    player->draw = gf3d_entity_draw;
    //player->update = gf3d_entity_update;
    */
    gfc_matrix_make_translation(
            modelMat2,
           vector3d(0,0,0)
        );



    gfc_matrix_rotate(
        modelMat2,
        modelMat2,
        90,
        vector3d(1,0,0));




    //gfc_matrix_make_translation(
    //    modelMat,
   //     vector3d(0,0,20)
   // );
    
    //gf3d_entity_draw(player);
    
    
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        //update game things here
        gf3d_entity_update_all();
        
        gf3d_entity_pre_sync_all();
        
        gf3d_entity_post_sync_all();
        
        gf3d_entity_think_all();
        
        
        
        
        //gf3d_vgraphics_rotate_camera(0.00005);
        
        if(keys == NULL){
        
            move = 0;
        }
        
        
       if(keys[SDL_SCANCODE_D]){
          
           
           //move -=0.0000025;
         
           move = 0.005;
           
           gfc_matrix_translate(
              modelMat2,
            vector3d(-move,0,0)
          );
           
           /*
           
           gfc_matrix_rotate(
                modelMat,
                modelMat,
                0.002,
                vector3d(0,0,1));
            */
           
           
           //important
           /*
           gfc_matrix_rotate(
                modelMat2,
                modelMat2,
                -0.002,
                vector3d(0,1,0));
        */
            
       }
       
       if(keys[SDL_SCANCODE_S]){
          
        
           //move -=0.00000025;
           
           
           move = 0.005;
           
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
        
          modelMat[3][0] = move;
          modelMat[3][1] = move;
          modelMat[3][2] = move;
          */
          gfc_matrix_translate(
              modelMat2,
            vector3d(0,move,0)
          );
       }
       if(keys[SDL_SCANCODE_W]){
          
        
           //move +=0.0000025;
           
           move = 0.005;
           
           /*
            gfc_matrix_make_translation(
            modelMat,
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
          gfc_matrix_translate(
              modelMat2,
            vector3d(0,-move,0)
          );
       }
       if(keys[SDL_SCANCODE_A]){
          
           //move +=0.0000025;
           
           move = 0.005;
           
         
           gfc_matrix_translate(
              modelMat2,
            vector3d(move,0,0)
          );
           
           
         
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
            
        move = 0;
        
        //gf3d_entity_draw_all();
        
        // configure render command for graphics command pool
        // for each mesh, get a command and configure it from the pool
        bufferFrame = gf3d_vgraphics_render_begin();
        gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),bufferFrame);
            commandBuffer = gf3d_command_rendering_begin(bufferFrame);
                frame +=0.025;
                if (frame>=56){
                 frame =0;
                 
                }
                
                
                
               gf3d_entity_draw_all(bufferFrame,frame,commandBuffer);
                
                
                
        //gf3d_model_draw(model,bufferFrame,commandBuffer,modelMat2,(Uint32)frame);
                //(Uint32)frame
                
               // player->model = model2;
               //  player->commandBuffer = commandBuffer;
              //  player->bufferFrame = bufferFrame;
              //  player->modelMat.strcmp(modelMat2);
        
       // gf3d_entity_draw(player);
    
//               gf3d_model_draw(model2,bufferFrame,commandBuffer,modelMat2);
                
            gf3d_command_rendering_end(commandBuffer);
            
        gf3d_vgraphics_render_end(bufferFrame);

        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
    }    
    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
