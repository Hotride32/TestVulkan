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
    VkCommandBuffer Buffer;
    Model *model = NULL;
    Matrix4 modelMat;
   // Model *model2 = NULL;
    Matrix4 modelMat2;
    Entity *player = NULL;
    Texture *texture = NULL;
    //gf3d_texture_load("images/dino.png");
    VkImage dstimage = NULL;
    Command * commandPool;
    
    
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
    //vector4d(0.51,0.75,1,1)
    
    
   gf3d_entity_manager_init(2028);
   
   
   //player->model = gf3d_model_load("dino");
   //player->bufferFrame
   
   player_spawn(vector3d(1,1,1));
   
   Vector3D skystate = vector3d(0,0,0);
   Vector3D skypos = vector3d(0,0,10000);
   
   monster_spawn(vector3d(-40,-40,0)); // changed from 7.5
   
   //snprintf(assetname,GFCLINELEN,"images/dino.png",filename);
   
    //texture = gf3d_texture_load("images/dino.png");
   
   
   //pickup_spawn(vector3d(10,0,0));
  // pickup_spawn(vector3d(0,0,10));
   //pickup_spawn(vector3d(-10,0,0));
   
   
   pickup_spawn(vector3d(10,0,0));
   
   
   //pickup_spawn(vector3d(0,0,-10));
   
   //player_spawn(vector3d(10,0,0));
    // main game loop
    slog("gf3d main loop begin");
    
    model = gf3d_model_load("skybox");
    
    
    //model = gf3d_model_load_animated("dun_idle",1,57);
    gfc_matrix_identity(modelMat);
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
//     gfc_matrix_make_translation(
//             modelMat2,
//            vector3d(0,0,0)
//         );


    //gfc_matrix_multiply(modelMat, modelMat, {1,1,1,10},{1,1,1,10},{1,1,1,10},{1,1,1,1});
/*
    modelMat[0][3] *= 100;
    modelMat[1][3] *= 100;
    modelMat[2][3] *= 100;
    */

    gfc_matrix_make_translation(
        modelMat,
        vector3d(0,0,10)
    );

    gfc_matrix_rotate(
        modelMat,
        modelMat,
        89.5,
        vector3d(1,0,0));


     //modelMat[0][3] += 100;
     //modelMat[1][3] += 100;
     //modelMat[2][3] += 100;
     
/*
    modelMat[3][0] += 100;
    modelMat[3][1] += 100;
    modelMat[3][2] += 100;*/
    
    modelMat[3][3] *= 0.005;
    /*
    gfc_matrix_make_translation(
        modelMat,
        vector3d(0,20,0)
    );*/
    
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
        
        //gf3d_swapchain_get_frame_buffer_by_index(index);
        
        
        //gf3d_vgraphics_rotate_camera(0.00005);
        
        //gf3d_entity_draw_all();
        
        // configure render command for graphics command pool
        // for each mesh, get a command and configure it from the pool
        bufferFrame = gf3d_vgraphics_render_begin();
        gf3d_pipeline_reset_frame(gf3d_vgraphics_get_graphics_pipeline(),bufferFrame);
        
        
        //commandPool = gf3d_vgraphics_get_graphics_command_pool();
      //Buffer = gf3d_command_begin_single_time(commandPool);
      //Texture *texture = gf3d_texture_load("images/dino.png");
     /*
      VkImageBlit region;
     region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
     region.srcSubresource.mipLevel = 0;
     region.srcSubresource.baseArrayLayer = 0;
     region.srcSubresource.layerCount = 1;
     region.srcOffsets[0].x = 0;
     region.srcOffsets[0].y = 0;
     region.srcOffsets[0].z = 0;
     region.srcOffsets[1].x = 32;
     region.srcOffsets[1].y = 32;
     region.srcOffsets[1].z = 1;
     region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
     region.dstSubresource.mipLevel = 0;
     region.dstSubresource.baseArrayLayer = 0;
     region.dstSubresource.layerCount = 1;
     region.dstOffsets[0].x = 0;
     region.dstOffsets[0].y = 0;
     region.dstOffsets[0].z = 0;
     region.dstOffsets[1].x = 1200;
     region.dstOffsets[1].y = 700;
     region.dstOffsets[1].z = 1;
 
     vkCmdBlitImage(Buffer, texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, model->texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &region, VK_FILTER_LINEAR);
     
    
     //gf3d_command_end_single_time(commandPool, Buffer);*/
        
     //commandBuffer = gf3d_command_rendering_begin(bufferFrame);
     
     //gf3d_swapchain_blit_to(commandBuffer,index);
     
     
     //gf3d_command_end_single_time(commandPool, Buffer);
     
     
            commandBuffer = gf3d_command_rendering_begin(bufferFrame);
            
                
                
                
               gf3d_entity_draw_all(bufferFrame,frame,commandBuffer);
                
                
        
                
        gf3d_model_draw(model,&skypos,&skystate,bufferFrame,commandBuffer,modelMat);
                //(Uint32)frame
                
               // player->model = model2;
               //  player->commandBuffer = commandBuffer;
              //  player->bufferFrame = bufferFrame;
              //  player->modelMat.strcmp(modelMat2);
        
       // gf3d_entity_draw(player);
    
               //gf3d_model_draw(model2,bufferFrame,commandBuffer,modelMat2);
                
            gf3d_command_rendering_end(commandBuffer);
    /*        
      commandPool = gf3d_vgraphics_get_graphics_command_pool();
      commandBuffer = gf3d_command_begin_single_time(commandPool);
      //Texture *texture = gf3d_texture_load("images/dino.png");
     
      VkImageBlit region;
     region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
     region.srcSubresource.mipLevel = 0;
     region.srcSubresource.baseArrayLayer = 0;
     region.srcSubresource.layerCount = 1;
     region.srcOffsets[0].x = 0;
     region.srcOffsets[0].y = 0;
     region.srcOffsets[0].z = 0;
     region.srcOffsets[1].x = 32;
     region.srcOffsets[1].y = 32;
     region.srcOffsets[1].z = 1;
     region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
     region.dstSubresource.mipLevel = 0;
     region.dstSubresource.baseArrayLayer = 0;
     region.dstSubresource.layerCount = 1;
     region.dstOffsets[0].x = 0;
     region.dstOffsets[0].y = 0;
     region.dstOffsets[0].z = 0;
     region.dstOffsets[1].x = 1200;
     region.dstOffsets[1].y = 700;
     region.dstOffsets[1].z = 1;
 
     vkCmdBlitImage(commandBuffer, texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, model->texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &region, VK_FILTER_LINEAR);
     
    
     gf3d_command_end_single_time(commandPool, commandBuffer);
            */
        gf3d_vgraphics_render_end(bufferFrame);

        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
    }    
    gf3d_entity_manager_close();
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
