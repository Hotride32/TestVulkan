
#include <string.h>
#include "simple_logger.h"

#include "gf3d_commands.h"
#include "gf3d_vgraphics.h"
#include "gf3d_vqueues.h"
#include "gf3d_swapchain.h"
#include "gf3d_mesh.h"
#include "gf3d_texture.h"
#include "entity3d.h"
#include "player3d.h"

 // TODO: Make a command buffer resource manager

 
typedef struct
{
    Command     *   command_list;
    Uint32          max_commands;
    VkDevice        device;
}CommandManager;


static CommandManager gf3d_commands = {0};

static int state = 0;


void gf3d_command_pool_close();
void gf3d_command_free(Command *com);
void gf3d_command_buffer_begin(Command *com,Pipeline *pipe);
void gf3d_command_configure_render_pass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,VkFramebuffer framebuffer,VkPipeline graphicsPipeline,VkPipelineLayout pipelineLayout);

void gf3d_command_system_close()
{
    int i;
    if (gf3d_commands.command_list != NULL)
    {
        for (i = 0; i < gf3d_commands.max_commands; i++)
        {
            gf3d_command_free(&gf3d_commands.command_list[i]);
        }
        free(gf3d_commands.command_list);
    }
    slog("command pool system closed");
}

void gf3d_command_system_init(Uint32 max_commands,VkDevice defaultDevice)
{
    slog("command pool system init");
    if (!max_commands)
    {
        slog("cannot initliaze 0 command pools");
        return;
    }
    gf3d_commands.device = defaultDevice;
    gf3d_commands.max_commands = max_commands;
    gf3d_commands.command_list = (Command*)gfc_allocate_array(sizeof(Command),max_commands);
    
    atexit(gf3d_command_system_close);
}

Command *gf3d_command_pool_new()
{
    int i;
    for (i = 0; i < gf3d_commands.max_commands;i++)
    {
        if (!gf3d_commands.command_list[i]._inuse)
        {
            gf3d_commands.command_list[i]._inuse = 1;
            return &gf3d_commands.command_list[i];
        }
    }
    slog("failed to get a new command pool, list full");
    return NULL;
}

void gf3d_command_free(Command *com)
{
    if ((!com)||(!com->_inuse))return;
    if (com->commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(gf3d_commands.device, com->commandPool, NULL);
    }
    if (com->commandBuffers)
    {
        free(com->commandBuffers);
    }
    memset(com,0,sizeof(Command));
}


Command * gf3d_command_graphics_pool_setup(Uint32 count,Pipeline *pipe)
{
    Command *com;
    VkCommandPoolCreateInfo poolInfo = {0};
    VkCommandBufferAllocateInfo allocInfo = {0};
    
    com = gf3d_command_pool_new();
    
    if (!com)
    {
        return NULL;
    }
    
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = gf3d_vqueues_get_graphics_queue_family();
    poolInfo.flags = 0; // Optional    
    
    if (vkCreateCommandPool(gf3d_commands.device, &poolInfo, NULL, &com->commandPool) != VK_SUCCESS)
    {
        slog("failed to create command pool!");
        return NULL;
    }
    
    com->commandBuffers = (VkCommandBuffer*)gfc_allocate_array(sizeof(VkCommandBuffer),count);
    if (!com->commandBuffers)
    {
        slog("failed to allocate command buffer array");
        gf3d_command_free(com);
        return NULL;
    }
    
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = com->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;
    com->commandBufferCount = count;

    if (vkAllocateCommandBuffers(gf3d_commands.device, &allocInfo, com->commandBuffers) != VK_SUCCESS)
    {
        slog("failed to allocate command buffers!");
        gf3d_command_free(com);
        return NULL;
    }
    
    slog("created command buffer pool");
    return com;
}

VkCommandBuffer * gf3d_command_pool_get_used_buffers(Command *com)
{
    if (!com)return NULL;
    return com->commandBuffers;
}

Uint32 gf3d_command_pool_get_used_buffer_count(Command *com)
{
    if (!com)return 0;
    return com->commandBufferNext;
}

void gf3d_command_pool_reset(Command *com)
{
    if (!com)return;
    com->commandBufferNext = 0;
}

VkCommandBuffer gf3d_command_get_graphics_buffer(Command *com)
{
    if (!com)return VK_NULL_HANDLE;
    if (com->commandBufferNext >= com->commandBufferCount)
    {
        slog("out of command buffers for the command pool");
        return VK_NULL_HANDLE;
    }
    return com->commandBuffers[com->commandBufferNext++];
}

void gf3d_command_configure_render_pass_end(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

VkCommandBuffer gf3d_command_rendering_begin(Uint32 index, int *done)
{
     int mousex,mousey;
    const Uint8 * keys;
     
    //int state;
    
    VkCommandBuffer commandBuffer;
    Pipeline *pipe;
    pipe = gf3d_vgraphics_get_graphics_pipeline();
    SDL_Event event;
    Rect button;
    Vector3D mousepoint;
    
    //Entity *player;
    //player = player_get();
    
    Texture *spawnm = gf3d_texture_load("images/spawnm.png");
    Texture *pointer = gf3d_texture_load("images/pointer.png");
    Texture *test = gf3d_texture_load("images/test.png");
    Texture *yes = gf3d_texture_load("images/yes.png");
    Texture *no = gf3d_texture_load("images/no.png");
    Texture *level1 = gf3d_texture_load("images/level1.png");
    Texture *level2 = gf3d_texture_load("images/level2.png");
    Texture *start = gf3d_texture_load("images/start.png");
    Texture *bg_flat = gf3d_texture_load("images/bg_flat.png");
    Texture *quit = gf3d_texture_load("images/quit.png");
    Texture *select = gf3d_texture_load("images/select.png");
    
    //Texture *text = gf3d_texture_load_text("images/dino_attack.png");
    
    SDL_PumpEvents();   // update SDL's internal event structures
        
        SDL_GetMouseState(&mousex,&mousey);
    
        //button.x = 0;
       // button.y = 0;
        //button.w = 0;
        //button.h = 0;
        
        
        mousepoint.x = mousex;
        mousepoint.y = mousey;
        
        SDL_ShowCursor(SDL_DISABLE);
    
    commandBuffer = gf3d_command_begin_single_time(gf3d_vgraphics_get_graphics_command_pool());
  
//     if(player != NULL){
//     
    //gf3d_swapchain_blit_to(commandBuffer,0,1024,0,1024,0,player->health, 0,200,texture);
//     }
    //else{
    keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_M]){
    
        button.x = 700;
        button.y = 0;
        button.w = 1200;
        button.h = 69;
        
        gf3d_swapchain_blit_to(commandBuffer,0,500,0,69,700, 1200, 0,69,spawnm);
        
        if(mousex+32 > 1200 || mousey+32 > 700){
        gf3d_swapchain_blit_to(commandBuffer,0,32,0,32,mousex, mousex, mousey,mousey,pointer);
        }
        else{
            gf3d_swapchain_blit_to(commandBuffer,0,32,0,32,mousex,32+mousex, mousey,32+mousey,pointer);
        }
        
    }
    
    
    
   gf3d_swapchain_blit_health(commandBuffer,0,50,0,50,0, 500, 0,200,test);
     
  // gf3d_swapchain_blit_to(commandBuffer,0,1200,0,700,0, 1200, 0,700,bg_flat);
//    gf3d_swapchain_blit_to(commandBuffer,0,111,0,61,700, 700+111, 500,500+61,no);
//    gf3d_swapchain_blit_to(commandBuffer,0,138,0,71,400, 400+138, 500,500+71,yes);
   //gf3d_swapchain_blit_to(commandBuffer,0,178,0,64,511, 511+178, 200,200+64,start);
   //gf3d_swapchain_blit_to(commandBuffer,0,212,0,65,300, 300+212, 350,350+65,level1);
  // gf3d_swapchain_blit_to(commandBuffer,0,235,0,66,700, 700+235, 350,350+66,level2);
   
   
   
   Rect startr;
   /*
        startr.x = 511;
        startr.y = 200;
        startr.w = 178;
        startr.h = 64;
   */
   Rect yesr;
   /*
        yesr.x = 400;
        yesr.y = 500;
        yesr.w = 138;
        yesr.h = 71;
   */
   Rect nor;
   /*
        nor.x = 700;
        nor.y = 500;
        nor.w = 111;
        nor.h = 61;
        */
   Rect level1r;
   /*
        level1r.x = 300;
        level1r.y = 350;
        level1r.w = 212;
        level1r.h = 65;*/
        
   Rect level2r;
   
//         level2r.x = 700;
//         level2r.y = 350;
//         level2r.w = 235;
//         level2r.h = 66;
   
   Rect quitr;
 /*       
        quitr.x = 400;
        quitr.y = 500;
        quitr.w = 164;
        quitr.h = 66;*/
   
   Rect selectr;
   /*
        selectr.x = 700;
        selectr.y = 500;
        selectr.w = 238;
        selectr.h = 43;*/
   
   if (keys[SDL_SCANCODE_ESCAPE]){
//       gf3d_swapchain_blit_to(commandBuffer,0,111,0,61,700, 700+111, 500,500+61,no);
//         gf3d_swapchain_blit_to(commandBuffer,0,138,0,71,400, 400+138, 500,500+71,yes);
//         yesr.x = 400;
//         yesr.y = 500;
//         yesr.w = 138;
//         yesr.h = 71;
//         nor.x = 700;
//         nor.y = 500;
//         nor.w = 111;
//         nor.h = 61;
       gf3d_swapchain_blit_to(commandBuffer,0,238,0,43,700, 700+138, 500,500+43,select);
        gf3d_swapchain_blit_to(commandBuffer,0,164,0,66,400, 400+164, 500,500+66,quit);
        
        selectr.x = 700;
        selectr.y = 500;
        selectr.w = 238;
        selectr.h = 43;
        
         quitr.x = 400;
        quitr.y = 500;
        quitr.w = 164;
        quitr.h = 66;
        
        if(mousex+32 > 1200 || mousey+32 > 700){
        gf3d_swapchain_blit_to(commandBuffer,0,32,0,32,mousex, mousex, mousey,mousey,pointer);
        }
        else{
            gf3d_swapchain_blit_to(commandBuffer,0,32,0,32,mousex,32+mousex, mousey,32+mousey,pointer);
        }
       
   }
   
   if(state < 2 && !keys[SDL_SCANCODE_ESCAPE]){ 
   
       gf3d_swapchain_blit_to(commandBuffer,0,1200,0,700,0, 1200, 0,700,bg_flat);
       
       
       
   }
   if(state == 0 && !keys[SDL_SCANCODE_ESCAPE]){
   
       
       
        startr.x = 511;
        startr.y = 200;
        startr.w = 178;
        startr.h = 64;   
       
        gf3d_swapchain_blit_to(commandBuffer,0,178,0,64,511, 511+178, 200,200+64,start);
        
    
       
   }
   if(state == 1 && !keys[SDL_SCANCODE_ESCAPE]){
        level1r.x = 300;
            level1r.y = 350;
            level1r.w = 212;
            level1r.h = 65;
            
            gf3d_swapchain_blit_to(commandBuffer,0,212,0,65,300, 300+212, 350,350+65,level1);
            
            level2r.x = 700;
            level2r.y = 350;
            level2r.w = 235;
            level2r.h = 66;
                
            gf3d_swapchain_blit_to(commandBuffer,0,235,0,66,700, 700+235, 350,350+66,level2);
        
    }
   if(state < 2 && !keys[SDL_SCANCODE_ESCAPE]){ 
   if(mousex+32 > 1200 || mousey+32 > 700){
        gf3d_swapchain_blit_to(commandBuffer,0,32,0,32,mousex, mousex, mousey,mousey,pointer);
        }
        else{
            gf3d_swapchain_blit_to(commandBuffer,0,32,0,32,mousex,32+mousex, mousey,32+mousey,pointer);
        }
   }
        
   if(SDL_PollEvent( &event )){
       if (keys[SDL_SCANCODE_ESCAPE]){
        
           
        if(gf3d_point_in_rect(mousepoint,quitr) && event.type == SDL_MOUSEBUTTONDOWN){
            //monster_spawn_at_player();
            state = 5;
            slog("YES button");
        }
        if(gf3d_point_in_rect(mousepoint,selectr) && event.type == SDL_MOUSEBUTTONDOWN){
            //monster_spawn_at_player();
            state = 0;
            entity_clear_all_but_player();
            
            slog("NO button");
        }
    }
       
        if(gf3d_point_in_rect(mousepoint,button) && event.type == SDL_MOUSEBUTTONDOWN && state >= 2){
            monster_spawn_at_player();
            slog("point in rect");
        }
        if(gf3d_point_in_rect(mousepoint,startr) && event.type == SDL_MOUSEBUTTONDOWN){
             
            state = 1;
            slog("Start button");
         }

        if(gf3d_point_in_rect(mousepoint,level1r) && event.type == SDL_MOUSEBUTTONDOWN){
            //gf3d_swapchain_blit_to(commandBuffer,0,212,0,65,300, 300+212, 350,350+65,level1);
            player_spawn(vector3d(1,1,1));
            pickup_spawn(vector3d(10,0,0));
            monster_spawn(vector3d(-40,-40,0));
            
            state = 2;
            slog("LEVEL 1 button");
        }
        if(gf3d_point_in_rect(mousepoint,level2r) && event.type == SDL_MOUSEBUTTONDOWN){
            //gf3d_swapchain_blit_to(commandBuffer,0,235,0,66,700, 700+235, 350,350+66,level2);
            state = 2;
            slog("LEVEL 2 button");
        }
        
    }
    
//     
    
   
   
    //gf3d_swapchain_blit_to(VkCommandBuffer commandBuffer,Uint32 srcWidthStart,Uint32 srcWidthEnd,Uint32 srcHeightStart,,Uint32 srcHeightEnd,Uint32 dstWidthStart,,Uint32 dstWidthEnd,Uint32 dstHeightStart,Uint32 dstHeightEnd,Texture *texture)
    
    
    
    gf3d_command_configure_render_pass(
            commandBuffer,
            pipe->renderPass,
            gf3d_swapchain_get_frame_buffer_by_index(index),
            pipe->pipeline,
            pipe->pipelineLayout);
    
    return commandBuffer;
}

int gf3d_command_rendering_end(VkCommandBuffer commandBuffer)
{
    gf3d_command_configure_render_pass_end(commandBuffer);
    
    //Texture *texture = gf3d_texture_load("images/dino.png");
    
    //Entity *player;
   // player = player_get();
    
     //gf3d_swapchain_blit_to(commandBuffer,0,1024,0,1024,0, player->health, 0,player->health,texture);
    
    
//     if (keys[SDL_SCANCODE_ESCAPE] && state < 2){
//         
//         gf3d_swapchain_blit_to(commandBuffer,0,111,0,61,700, 700+111, 500,500+61,no);
//         gf3d_swapchain_blit_to(commandBuffer,0,138,0,71,400, 400+138, 500,500+71,yes);
//         yesr.x = 400;
//         yesr.y = 500;
//         yesr.w = 138;
//         yesr.h = 71;
//         nor.x = 700;
//         nor.y = 500;
//         nor.w = 111;
//         nor.h = 61;
//         
//         if(gf3d_point_in_rect(mousepoint,yesr) && event.type == SDL_MOUSEBUTTONDOWN){
//             //monster_spawn_at_player();
//             done = 1;
//             slog("YES button");
//         }
//         if(gf3d_point_in_rect(mousepoint,nor) && event.type == SDL_MOUSEBUTTONDOWN){
//             //monster_spawn_at_player();
//             
//             slog("NO button");
//         }
//     }
//     
    
    
     
     
    gf3d_command_end_single_time(gf3d_vgraphics_get_graphics_command_pool(), commandBuffer);
    
    return state;
}

void gf3d_command_configure_render_pass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,VkFramebuffer framebuffer,VkPipeline graphicsPipeline,VkPipelineLayout pipelineLayout)
{
    VkClearValue clearValues[2] = {0};
    VkRenderPassBeginInfo renderPassInfo = {0};
    
    clearValues[0].color.float32[3] = 1.0;
    clearValues[1].depthStencil.depth = 1.0f;
    
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = gf3d_swapchain_get_extent();
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

VkCommandBuffer gf3d_command_begin_single_time(Command* com)
{
    VkCommandBufferAllocateInfo allocInfo = {0};
    VkCommandBufferBeginInfo beginInfo = {0};
    VkCommandBuffer commandBuffer;
    
    if (!com)
    {
        slog("com is NULL");
    }
    
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = com->commandPool;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(gf3d_commands.device, &allocInfo, &commandBuffer);

    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void gf3d_command_end_single_time(Command *com, VkCommandBuffer commandBuffer)
{
    VkSubmitInfo submitInfo = {0};
    
    vkEndCommandBuffer(commandBuffer);

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(gf3d_vqueues_get_graphics_queue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(gf3d_vqueues_get_graphics_queue());

    vkFreeCommandBuffers(gf3d_commands.device, com->commandPool, 1, &commandBuffer);
}

/*eol@eof*/
