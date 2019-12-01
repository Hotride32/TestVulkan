
#include <string.h>
#include "simple_logger.h"

#include "gf3d_commands.h"
#include "gf3d_vgraphics.h"
#include "gf3d_vqueues.h"
#include "gf3d_swapchain.h"
#include "gf3d_mesh.h"
#include "gf3d_texture.h"

 // TODO: Make a command buffer resource manager

 
typedef struct
{
    Command     *   command_list;
    Uint32          max_commands;
    VkDevice        device;
}CommandManager;


static CommandManager gf3d_commands = {0};

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

VkCommandBuffer gf3d_command_rendering_begin(Uint32 index)
{
    VkCommandBuffer commandBuffer;
    Pipeline *pipe;
    pipe = gf3d_vgraphics_get_graphics_pipeline();
    
    commandBuffer = gf3d_command_begin_single_time(gf3d_vgraphics_get_graphics_command_pool());
  
    //vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    
     gf3d_swapchain_blit_to(commandBuffer,index);
    
     
     //now in swapchain
    /*  
    Texture *texture = gf3d_texture_load("images/dino.png");
    
    VkImage srcImage = texture->textureImage;
    
    //VkExtent2D dstextent = textureImageView;
    
    VkImage dstImage = NULL;
    
    //vSwapChain swap = gf3d_swapchain_get_images(index);
    
    dstImage = gf3d_swapchain_get_images(index);
    
    if(dstImage == NULL){
        slog("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
    }
    
    VkExtent2D dstextent = gf3d_swapchain_get_extent();
    
    //texture.TextureImage;
    
    //slog("dstImage file %s",dstImage);
    
    slog("dstImage width %i",dstextent.width);
    slog("dstImage height %i",dstextent.height);
    
    VkImageMemoryBarrier barrier = {0};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = srcImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

      
    VkImageMemoryBarrier barrier2 = {0};
        barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier2.image = dstImage;
        barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier2.subresourceRange.baseArrayLayer = 0;
        barrier2.subresourceRange.layerCount = 1;
        barrier2.subresourceRange.levelCount = 1;    
        
        barrier2.subresourceRange.baseMipLevel = 0;
            barrier2.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier2.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier2.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        
        int mipWidth = dstextent.width ;
        int mipHeight = dstextent.height;
        //int mipLevels = 1;
        int Width = 819 ;
        int Height = 614 ;
       
        copyCmd,
			srcImage,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
        
       
        //for (Uint32 i = 1; i < mipLevels; i++) {
            //barrier.subresourceRange.baseMipLevel = i - 1;
          
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier);

            
            
               
    VkImageMemoryBarrier barrier2 = {0};
        barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier2.image = dstImage;
        barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier2.subresourceRange.baseMipLevel = 0;
        barrier2.subresourceRange.levelCount = 1;
        barrier2.subresourceRange.baseArrayLayer = 0;
        barrier2.subresourceRange.layerCount = 1;
            barrier2.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier2.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier2);
            
           
            
             VkImageBlit blit = {0};
             
     
             blit.srcOffsets[0].x = 0;
             blit.srcOffsets[0].y = 0;
             blit.srcOffsets[0].z = 0;
         blit.srcOffsets[1].x = Width;
         blit.srcOffsets[1].y = Height;
             //blit.srcOffsets[1].x = 32;
             //blit.srcOffsets[1].y = 32;
             blit.srcOffsets[1].z = 1;
             blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
              
   // blit.srcSubresource.baseMipLevel = 0;
    //blit.srcSubresource.levelCount = 1;
   // blit.srcSubresource.baseArrayLayer = 0;
    //blit.srcSubresource.layerCount = 1;
             
             blit.srcSubresource.mipLevel = 0;
             blit.srcSubresource.baseArrayLayer = 0;
             blit.srcSubresource.layerCount = 1;
             blit.dstOffsets[0].x = 0;
             blit.dstOffsets[0].y = 0;
             blit.dstOffsets[0].z = 0;
        
             blit.dstOffsets[1].x = mipWidth;
             blit.dstOffsets[1].y = mipHeight;
             blit.dstOffsets[1].z = 1 ;
             blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
             
            //blit.dstSubresource.baseMipLevel = 0;
           // blit.dstSubresource.levelCount = 1;
            //blit.dstSubresource.baseArrayLayer = 0;
            //blit.dstSubresource.layerCount = 1;
             
             
             blit.dstSubresource.mipLevel = 0;
             blit.dstSubresource.baseArrayLayer = 0;
             blit.dstSubresource.layerCount = 1;
            
             
             //something wrong with blit
            vkCmdBlitImage(commandBuffer,
                srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);
            
            
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier);

            
            barrier2.subresourceRange.baseMipLevel = 0;
            barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier2);
            
            //if (mipWidth > 1) mipWidth /= 2;
            //if (mipHeight > 1) mipHeight /= 2;
        //}

        //barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, NULL,
            0, NULL,
            1, &barrier);
    */
    
    //gf3d_swapchain_transition_image_layout(dstImage,VK_FORMAT_B8G8R8A8_UNORM,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    //gf3d_swapchain_transition_image_layout(dstImage,VK_FORMAT_B8G8R8A8_UNORM,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_UNDEFINED);
    
    
    gf3d_command_configure_render_pass(
            commandBuffer,
            pipe->renderPass,
            gf3d_swapchain_get_frame_buffer_by_index(index),
            pipe->pipeline,
            pipe->pipelineLayout);
    
    return commandBuffer;
}

void gf3d_command_rendering_end(VkCommandBuffer commandBuffer)
{
    gf3d_command_configure_render_pass_end(commandBuffer);
    gf3d_command_end_single_time(gf3d_vgraphics_get_graphics_command_pool(), commandBuffer);
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
