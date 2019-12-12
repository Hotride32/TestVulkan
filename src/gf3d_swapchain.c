#include <string.h>
#include <stdio.h>

#include "simple_logger.h"

#include "gf3d_swapchain.h"
#include "gf3d_vqueues.h"
#include "gf3d_vgraphics.h"
#include "gf3d_texture.h"
#include "entity3d.h"
#include "player3d.h"

typedef struct
{
    VkDevice                    device;
    VkSurfaceCapabilitiesKHR    capabilities;
    Uint32                      formatCount;
    VkSurfaceFormatKHR         *formats;
    Uint32                      presentModeCount;
    VkPresentModeKHR           *presentModes;
    int                         chosenFormat;
    int                         chosenPresentMode;
    VkExtent2D                  extent;                 // resolution of the swap buffers
    Uint32                      swapChainCount;
    VkSwapchainKHR              swapChain;
    VkImage                    *swapImages;
    Uint32                      swapImageCount;
    VkImageView                *imageViews;
    VkFramebuffer              *frameBuffers;
    Uint32                      framebufferCount;
    VkImage                     depthImage;
    VkDeviceMemory              depthImageMemory;
    VkImageView                 depthImageView;
}vSwapChain;

static vSwapChain gf3d_swapchain = {0};

void gf3d_swapchain_create(VkDevice device,VkSurfaceKHR surface);
void gf3d_swapchain_close();
int gf3d_swapchain_choose_format();
void gf3d_swapchain_create_depth_image();
VkImage *gf3d_swapchain_get_images(Uint32 index);

void gf3d_swapchain_blit_to(VkCommandBuffer commandBuffer,Uint32 srcWidthStart,Uint32 srcWidthEnd,Uint32 srcHeightStart,Uint32 srcHeightEnd,Uint32 dstWidthStart,Uint32 dstWidthEnd,Uint32 dstHeightStart,Uint32 dstHeightEnd,Texture *texture);

void gf3d_swapchain_blit_health(VkCommandBuffer commandBuffer,Uint32 srcWidthStart,Uint32 srcWidthEnd,Uint32 srcHeightStart,Uint32 srcHeightEnd,Uint32 dstWidthStart,Uint32 dstWidthEnd,Uint32 dstHeightStart,Uint32 dstHeightEnd,Texture *texture);

int gf3d_swapchain_get_presentation_mode();
VkExtent2D gf3d_swapchain_configure_extent(Uint32 width,Uint32 height);
uint32_t gf3d_swapchain_find_Memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void gf3d_swapchain_init(VkPhysicalDevice device,VkDevice logicalDevice,VkSurfaceKHR surface,Uint32 width,Uint32 height)
{
    int i;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &gf3d_swapchain.capabilities);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &gf3d_swapchain.formatCount, NULL);

    slog("device supports %i surface formats",gf3d_swapchain.formatCount);
    if (gf3d_swapchain.formatCount != 0)
    {
        gf3d_swapchain.formats = (VkSurfaceFormatKHR*)gfc_allocate_array(sizeof(VkSurfaceFormatKHR),gf3d_swapchain.formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &gf3d_swapchain.formatCount, gf3d_swapchain.formats);
        for (i = 0; i < gf3d_swapchain.formatCount; i++)
        {
            slog("surface format %i:",i);
            slog("format: %i",gf3d_swapchain.formats[i].format);
            slog("colorspace: %i",gf3d_swapchain.formats[i].colorSpace);
        }
    }
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &gf3d_swapchain.presentModeCount, NULL);

    slog("device supports %i presentation modes",gf3d_swapchain.presentModeCount);
    if (gf3d_swapchain.presentModeCount != 0)
    {
        gf3d_swapchain.presentModes = (VkPresentModeKHR*)gfc_allocate_array(sizeof(VkPresentModeKHR),gf3d_swapchain.presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &gf3d_swapchain.presentModeCount, gf3d_swapchain.presentModes);
        for (i = 0; i < gf3d_swapchain.presentModeCount; i++)
        {
            slog("presentation mode: %i is %i",i,gf3d_swapchain.presentModes[i]);
        }
    }
    
    gf3d_swapchain.chosenFormat = gf3d_swapchain_choose_format();
    slog("chosing surface format %i",gf3d_swapchain.chosenFormat);
    
    gf3d_swapchain.chosenPresentMode = gf3d_swapchain_get_presentation_mode();
    slog("chosing presentation mode %i",gf3d_swapchain.chosenPresentMode);
    
    gf3d_swapchain.extent = gf3d_swapchain_configure_extent(width,height);
    slog("chosing swap chain extent of (%i,%i)",gf3d_swapchain.extent.width,gf3d_swapchain.extent.height);
    
    gf3d_swapchain_create(logicalDevice,surface);
    gf3d_swapchain.device = logicalDevice;
    
    atexit(gf3d_swapchain_close);
}

void gf3d_swapchain_create_frame_buffer(VkFramebuffer *buffer,VkImageView *imageView,Pipeline *pipe)
{
    VkFramebufferCreateInfo framebufferInfo = {0};
    VkImageView imageViews[2];
    
    imageViews[0] = *imageView;
    imageViews[1] = gf3d_swapchain.depthImageView;

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = pipe->renderPass;
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = imageViews;
    framebufferInfo.width = gf3d_swapchain.extent.width;
    framebufferInfo.height = gf3d_swapchain.extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(gf3d_swapchain.device, &framebufferInfo, NULL, buffer) != VK_SUCCESS)
    {
        slog("failed to create framebuffer!");
    }
    else
    {
        slog("created framebuffer");
    }
}

void gf3d_swapchain_setup_frame_buffers(Pipeline *pipe)
{
    int i;
    gf3d_swapchain.frameBuffers = (VkFramebuffer *)gfc_allocate_array(sizeof(VkFramebuffer),gf3d_swapchain.swapImageCount);
    for (i = 0; i < gf3d_swapchain.swapImageCount;i++)
    {
        gf3d_swapchain_create_frame_buffer(&gf3d_swapchain.frameBuffers[i],&gf3d_swapchain.imageViews[i],pipe);
    }
    gf3d_swapchain.framebufferCount = gf3d_swapchain.swapImageCount;
}

VkFormat gf3d_swapchain_get_format()
{
    return gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format;
}

void gf3d_swapchain_create(VkDevice device,VkSurfaceKHR surface)
{
    int i;
    Sint32 graphicsFamily;
    Sint32 presentFamily;
    Sint32 transferFamily;
    VkSwapchainCreateInfoKHR createInfo = {0};
    Uint32 queueFamilyIndices[3];
    
    
    slog("minimum images needed for swap chain: %i",gf3d_swapchain.capabilities.minImageCount);
    slog("Maximum images needed for swap chain: %i",gf3d_swapchain.capabilities.maxImageCount);
    gf3d_swapchain.swapChainCount = gf3d_swapchain.capabilities.minImageCount + 1;
    if (gf3d_swapchain.capabilities.maxImageCount)gf3d_swapchain.swapChainCount = MIN(gf3d_swapchain.swapChainCount,gf3d_swapchain.capabilities.maxImageCount);
    slog("using %i images for the swap chain",gf3d_swapchain.swapChainCount);
    
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = gf3d_swapchain.swapChainCount;
    createInfo.imageFormat = gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format;
    createInfo.imageColorSpace = gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].colorSpace;
    createInfo.imageExtent = gf3d_swapchain.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    graphicsFamily = gf3d_vqueues_get_graphics_queue_family();
    presentFamily = gf3d_vqueues_get_present_queue_family();
    transferFamily = gf3d_vqueues_get_transfer_queue_family();
    queueFamilyIndices[0] = graphicsFamily;
    queueFamilyIndices[1] = presentFamily;
    queueFamilyIndices[2] = transferFamily;
    
    if (graphicsFamily != presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 3;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.preTransform = gf3d_swapchain.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // our window is opaque, but it doesn't have to be

    createInfo.presentMode = gf3d_swapchain.presentModes[gf3d_swapchain.chosenPresentMode];
    createInfo.clipped = VK_TRUE;
    
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &gf3d_swapchain.swapChain) != VK_SUCCESS)
    {
        slog("failed to create swap chain!");
        gf3d_swapchain_close();
        return;
    }
    slog("created a swap chain with length %i",gf3d_swapchain.swapChainCount);
    
    vkGetSwapchainImagesKHR(device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount, NULL);
    if (gf3d_swapchain.swapImageCount == 0)
    {
        slog("failed to create any swap images!");
        gf3d_swapchain_close();
        return;
    }
    gf3d_swapchain.swapImages = (VkImage *)gfc_allocate_array(sizeof(VkImage),gf3d_swapchain.swapImageCount);
    vkGetSwapchainImagesKHR(device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount,gf3d_swapchain.swapImages );
    slog("created swap chain with %i images",gf3d_swapchain.swapImageCount);
    
//     VkCommandBuffer commandBuffer = gf3d_command_begin_single_time(gf3d_vgraphics_get_graphics_command_pool());
//   
//     gf3d_swapchain_blit_to(commandBuffer,1);
//     
//     gf3d_command_end_single_time(gf3d_vgraphics_get_graphics_command_pool(), commandBuffer);
    
    gf3d_swapchain.imageViews = (VkImageView *)gfc_allocate_array(sizeof(VkImageView),gf3d_swapchain.swapImageCount);
    
    for (i = 0 ; i < gf3d_swapchain.swapImageCount; i++)
    {
        gf3d_swapchain.imageViews[i] = gf3d_vgraphics_create_image_view(gf3d_swapchain.swapImages[i],gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format,1);
    }
    slog("create image views");
}

VkExtent2D gf3d_swapchain_configure_extent(Uint32 width,Uint32 height)
{
    VkExtent2D actualExtent;
    slog("Requested resolution: (%i,%i)",width,height);
    slog("Minimum resolution: (%i,%i)",gf3d_swapchain.capabilities.minImageExtent.width,gf3d_swapchain.capabilities.minImageExtent.height);
    slog("Maximum resolution: (%i,%i)",gf3d_swapchain.capabilities.maxImageExtent.width,gf3d_swapchain.capabilities.maxImageExtent.height);
    
    actualExtent.width = MAX(gf3d_swapchain.capabilities.minImageExtent.width,MIN(width,gf3d_swapchain.capabilities.maxImageExtent.width));
    actualExtent.height = MAX(gf3d_swapchain.capabilities.minImageExtent.height,MIN(height,gf3d_swapchain.capabilities.maxImageExtent.height));
    return actualExtent;
}

VkExtent2D gf3d_swapchain_get_extent()
{
    return gf3d_swapchain.extent;
}


int gf3d_swapchain_get_presentation_mode()
{
    int i;
    int chosen = -1;
    for (i = 0; i < gf3d_swapchain.formatCount; i++)
    {
        if (gf3d_swapchain.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return i;
        chosen = i;
    }
    return chosen;
}

int gf3d_swapchain_choose_format()
{
    int i;
    int chosen = -1;
    for (i = 0; i < gf3d_swapchain.formatCount; i++)
    {
        if ((gf3d_swapchain.formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
            (gf3d_swapchain.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
            return i;
        chosen = i;
    }
    return chosen;
}

void gf3d_swapchain_close()
{
    int i;
    slog("cleaning up swapchain");
    
    if (gf3d_swapchain.depthImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gf3d_swapchain.device, gf3d_swapchain.depthImageView, NULL);
    }
    if (gf3d_swapchain.depthImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(gf3d_swapchain.device, gf3d_swapchain.depthImage, NULL);
    }
    if (gf3d_swapchain.depthImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_swapchain.device, gf3d_swapchain.depthImageMemory, NULL);
    }
    if (gf3d_swapchain.frameBuffers)
    {
        for (i = 0;i < gf3d_swapchain.framebufferCount; i++)
        {
            vkDestroyFramebuffer(gf3d_swapchain.device, gf3d_swapchain.frameBuffers[i], NULL);
            slog("framebuffer destroyed");
        }
        free (gf3d_swapchain.frameBuffers);
    }
    vkDestroySwapchainKHR(gf3d_swapchain.device, gf3d_swapchain.swapChain, NULL);
    if (gf3d_swapchain.imageViews)
    {
        for (i = 0;i < gf3d_swapchain.swapImageCount;i++)
        {
            vkDestroyImageView(gf3d_swapchain.device,gf3d_swapchain.imageViews[i],NULL);
            slog("imageview destroyed");
        }
        free(gf3d_swapchain.imageViews);
    }
    if (gf3d_swapchain.swapImages)
    {
        free(gf3d_swapchain.swapImages);
    }
    if (gf3d_swapchain.formats)
    {
        free(gf3d_swapchain.formats);
    }
    if (gf3d_swapchain.presentModes)
    {
        free(gf3d_swapchain.presentModes);
    }
    memset(&gf3d_swapchain,0,sizeof(vSwapChain));
}

Bool gf3d_swapchain_validation_check()
{
    if (!gf3d_swapchain.presentModeCount)
    {
        slog("swapchain has no usable presentation modes");
        return false;
    }
    if (!gf3d_swapchain.formatCount)
    {
        slog("swapchain has no usable surface formats");
        return false;
    }
    return true;
}

Uint32 gf3d_swapchain_get_chain_length()
{
    return gf3d_swapchain.swapChainCount;
}

Uint32 gf3d_swapchain_get_swap_image_count()
{
    return gf3d_swapchain.swapImageCount;
}

Uint32 gf3d_swapchain_get_frame_buffer_count()
{
    return gf3d_swapchain.framebufferCount;
}

VkSwapchainKHR gf3d_swapchain_get()
{
    return gf3d_swapchain.swapChain;
}

VkFramebuffer gf3d_swapchain_get_frame_buffer_by_index(Uint32 index)
{
    if (index >= gf3d_swapchain.framebufferCount)
    {
        slog("FATAL: index for framebuffer out of range");
        return 0;
    }
    return gf3d_swapchain.frameBuffers[index];
}

VkImageView gf3d_swapchain_create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = {0};
    VkImageView imageView;
    /*
    //added
    Command * commandPool;
    VkCommandBuffer commandBuffer;
    Texture *texture = gf3d_texture_load("images/dino.png");
    commandPool = gf3d_vgraphics_get_graphics_command_pool();
    commandBuffer = gf3d_command_begin_single_time(commandPool);
     gf3d_command_end_single_time(commandPool, commandBuffer);
    
    
     VkOffset3D blitSize;
			blitSize.x = 200;
			blitSize.y = 100;
			blitSize.z = 1;
			VkImageBlit imageBlitRegion = {0};
			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.srcSubresource.layerCount = 1;
			imageBlitRegion.srcOffsets[1] = blitSize;
			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.dstSubresource.layerCount = 1;
			imageBlitRegion.dstOffsets[1] = blitSize;
            
            vkCmdBlitImage(
				commandBuffer,
				texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlitRegion,
				VK_FILTER_NEAREST);
    
    
    //end of added
     */
     
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(gf3d_swapchain.device, &viewInfo, NULL, &imageView) != VK_SUCCESS)
    {
        slog("failed to create texture image view!");
        return VK_NULL_HANDLE;
    }

    return imageView;
}
    
Uint8 gf3d_swapchain_has_stencil_component(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
    
void gf3d_swapchain_transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier = {0};
    Command * commandPool;
    VkCommandBuffer commandBuffer;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (gf3d_swapchain_has_stencil_component(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        slog("unsupported layout transition!");
    }
    
    commandPool = gf3d_vgraphics_get_graphics_command_pool();
    commandBuffer = gf3d_command_begin_single_time(commandPool);
    /*Texture *texture = gf3d_texture_load("images/dino.png");
    
     VkOffset3D blitSize;
 			blitSize.x = 200;
 			blitSize.y = 100;
 			blitSize.z = 1;
 			VkImageBlit imageBlitRegion = {0};
 			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
 			imageBlitRegion.srcSubresource.layerCount = 1;
 			imageBlitRegion.srcOffsets[1] = blitSize;
 			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
 			imageBlitRegion.dstSubresource.layerCount = 1;
 			imageBlitRegion.dstOffsets[1] = blitSize;
             
             vkCmdBlitImage(
 				commandBuffer,
 				texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
 				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 				1,
 				&imageBlitRegion,
 				VK_FILTER_NEAREST);
    
    
    */
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier);
   
    gf3d_command_end_single_time(commandPool, commandBuffer);
    
    //added
//     Command * commandPool;
//     VkCommandBuffer commandBuffer;
//     Texture *texture = gf3d_texture_load("images/dino.png");
//     commandPool = gf3d_vgraphics_get_graphics_command_pool();
//     commandBuffer = gf3d_command_begin_single_time(commandPool);
//      gf3d_command_end_single_time(commandPool, commandBuffer);
//     
//     
//      VkOffset3D blitSize;
// 			blitSize.x = 200;
// 			blitSize.y = 100;
// 			blitSize.z = 1;
// 			VkImageBlit imageBlitRegion = {0};
// 			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			imageBlitRegion.srcSubresource.layerCount = 1;
// 			imageBlitRegion.srcOffsets[1] = blitSize;
// 			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			imageBlitRegion.dstSubresource.layerCount = 1;
// 			imageBlitRegion.dstOffsets[1] = blitSize;
//             
//             vkCmdBlitImage(
// 				commandBuffer,
// 				texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
// 				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// 				1,
// 				&imageBlitRegion,
// 				VK_FILTER_NEAREST);
//     
//     
    //end of added
    
}

void gf3d_swapchain_create_depth_image()
{
    gf3d_swapchain_create_image(gf3d_swapchain.extent.width, gf3d_swapchain.extent.height, gf3d_pipeline_find_depth_format(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gf3d_swapchain.depthImage, &gf3d_swapchain.depthImageMemory);
    /*
     Command * commandPool;
    VkCommandBuffer commandBuffer;
    Texture *texture = gf3d_texture_load("images/dino.png");
    commandPool = gf3d_vgraphics_get_graphics_command_pool();
    commandBuffer = gf3d_command_begin_single_time(commandPool);
     gf3d_command_end_single_time(commandPool, commandBuffer);
    
    
     VkOffset3D blitSize;
			blitSize.x = 200;
			blitSize.y = 100;
			blitSize.z = 1;
			VkImageBlit imageBlitRegion = {};
			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.srcSubresource.layerCount = 1;
			imageBlitRegion.srcOffsets[1] = blitSize;
			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.dstSubresource.layerCount = 1;
			imageBlitRegion.dstOffsets[1] = blitSize;
            
            vkCmdBlitImage(
				commandBuffer,
				texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				gf3d_swapchain.depthImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlitRegion,
				VK_FILTER_NEAREST);
    
    
    //end of added*/
    
    gf3d_swapchain.depthImageView = gf3d_swapchain_create_image_view(gf3d_swapchain.depthImage, gf3d_pipeline_find_depth_format(),VK_IMAGE_ASPECT_DEPTH_BIT);
    
    
    gf3d_swapchain_transition_image_layout(gf3d_swapchain.depthImage, gf3d_pipeline_find_depth_format(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    
}

uint32_t gf3d_swapchain_find_Memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    uint32_t i;
    VkPhysicalDeviceMemoryProperties memProperties;
    
    vkGetPhysicalDeviceMemoryProperties(gf3d_vgraphics_get_default_physical_device(), &memProperties);

    for (i= 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    slog("failed to find suitable memory type!");
    return 0;
}

void gf3d_swapchain_create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory)
{
    VkImageCreateInfo imageInfo = {0};
    VkMemoryAllocateInfo allocInfo = {0};
    VkMemoryRequirements memRequirements;
    
    //Command * commandPool;
    //VkCommandBuffer commandBuffer;
    

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    
    
    
//      commandPool = gf3d_vgraphics_get_graphics_command_pool();
//      commandBuffer = gf3d_command_begin_single_time(commandPool);
//      Texture *texture = gf3d_texture_load("images/dino.png");
//     
//      VkImageBlit region;
//     region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     region.srcSubresource.mipLevel = 0;
//     region.srcSubresource.baseArrayLayer = 0;
//     region.srcSubresource.layerCount = 1;
//     region.srcOffsets[0].x = 0;
//     region.srcOffsets[0].y = 0;
//     region.srcOffsets[0].z = 0;
//     region.srcOffsets[1].x = 32;
//     region.srcOffsets[1].y = 32;
//     region.srcOffsets[1].z = 1;
//     region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     region.dstSubresource.mipLevel = 0;
//     region.dstSubresource.baseArrayLayer = 0;
//     region.dstSubresource.layerCount = 1;
//     region.dstOffsets[0].x = 0;
//     region.dstOffsets[0].y = 0;
//     region.dstOffsets[0].z = 0;
//     region.dstOffsets[1].x = width;
//     region.dstOffsets[1].y = height;
//     region.dstOffsets[1].z = 1;
// 
//     vkCmdBlitImage(commandBuffer, texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                    1, &region, VK_FILTER_LINEAR);
//     
//    
//     gf3d_command_end_single_time(commandPool, commandBuffer);

    if (vkCreateImage(gf3d_swapchain.device, &imageInfo, NULL, image) != VK_SUCCESS)
    {
        slog("failed to create image!");
    }
   
    vkGetImageMemoryRequirements(gf3d_swapchain.device, *image, &memRequirements);

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = gf3d_swapchain_find_Memory_type(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(gf3d_swapchain.device, &allocInfo, NULL, imageMemory) != VK_SUCCESS)
    {
        slog("failed to allocate image memory!");
    }

    vkBindImageMemory(gf3d_swapchain.device, *image, *imageMemory, 0);
    
//      Command * commandPool = gf3d_vgraphics_get_graphics_command_pool();
//       VkCommandBuffer commandBuffer = gf3d_command_begin_single_time(commandPool);
//      
//         VkImageMemoryBarrier barrier = {0};
//         barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//         barrier.image = *image;
//         barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         barrier.subresourceRange.baseArrayLayer = 0;
//         barrier.subresourceRange.layerCount = 1;
//         barrier.subresourceRange.levelCount = 1;
// 
//         int mipWidth = width;
//         int mipHeight = height;
//         int mipLevels = 2;
//         
//         
//         for (Uint32 i = 1; i < mipLevels; i++) {
//             barrier.subresourceRange.baseMipLevel = i - 1;
//             barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//             barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//             barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 
//             vkCmdPipelineBarrier(commandBuffer,
//                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
//                 0, NULL,
//                 0, NULL,
//                 1, &barrier);
// 
//             VkImageBlit blit = {0};
//             blit.srcOffsets[0].x = 0;
//             blit.srcOffsets[0].y = 0;
//             blit.srcOffsets[0].z = 0;
//             blit.srcOffsets[1].x = mipWidth;
//             blit.srcOffsets[1].y = mipHeight;
//             blit.srcOffsets[1].z = 1;
//             blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//             blit.srcSubresource.mipLevel = i - 1;
//             blit.srcSubresource.baseArrayLayer = 0;
//             blit.srcSubresource.layerCount = 1;
//             blit.dstOffsets[0].x = 0;
//             blit.dstOffsets[0].y = 0;
//             blit.dstOffsets[0].z = 0;
//             blit.dstOffsets[1].x = mipWidth > 1 ? mipWidth / 2 : 1;
//             blit.dstOffsets[1].y = mipHeight > 1 ? mipHeight / 2 : 1;
//             blit.dstOffsets[1].z = 1 ;
//             blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//             blit.dstSubresource.mipLevel = i;
//             blit.dstSubresource.baseArrayLayer = 0;
//             blit.dstSubresource.layerCount = 1;
// 
//             vkCmdBlitImage(commandBuffer,
//                 *image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                 *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                 1, &blit,
//                 VK_FILTER_LINEAR);
// 
//             barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//             barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 
//             vkCmdPipelineBarrier(commandBuffer,
//                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
//                 0, NULL,
//                 0, NULL,
//                 1, &barrier);
// 
//             if (mipWidth > 1) mipWidth /= 2;
//             if (mipHeight > 1) mipHeight /= 2;
//         }
// 
//         barrier.subresourceRange.baseMipLevel = mipLevels - 1;
//         barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//         barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 
//         vkCmdPipelineBarrier(commandBuffer,
//             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
//             0, NULL,
//             0, NULL,
//             1, &barrier);
// 
//         gf3d_command_end_single_time(commandPool,commandBuffer);
    
}
VkImage *gf3d_swapchain_get_images(Uint32 index){
    
    VkImage image = gf3d_swapchain.swapImages[index];
    
   return image;
}


void gf3d_swapchain_blit_to(VkCommandBuffer commandBuffer,Uint32 srcWidthStart,Uint32 srcWidthEnd,Uint32 srcHeightStart,Uint32 srcHeightEnd,Uint32 dstWidthStart,Uint32 dstWidthEnd,Uint32 dstHeightStart,Uint32 dstHeightEnd,Texture *texture){

    VkImageSubresourceRange ImageSubresourceRange;
   ImageSubresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   ImageSubresourceRange.baseMipLevel   = 0;
   ImageSubresourceRange.levelCount     = 1;
   ImageSubresourceRange.baseArrayLayer = 0;
   ImageSubresourceRange.layerCount     = 1;

   VkClearColorValue ClearColorValue = {{ 1.0, 0.0, 0.0, 0.0 }};

   for (int i = 0; i < gf3d_swapchain.swapImageCount; i++)
   {
      //VkCommandBuffer CommandBuffer = CommandBuffers[i];

      //Result = vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);

      //PrintVkResult("vkBeginCommandBuffer", Result);

      //vkCmdClearColorImage(commandBuffer, gf3d_swapchain.swapImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColorValue, 1, &ImageSubresourceRange);

     // Result = vkEndCommandBuffer(CommandBuffer);

      //PrintVkResult("vkEndCommandBuffer", Result);
   //}
    
    
    
    //Texture *texture = gf3d_texture_load("images/dino.png");
    
    VkImage srcImage = texture->textureImage;
    
    //vkCmdClearColorImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColorValue, 1, &ImageSubresourceRange);
    
    
    //VkExtent2D dstextent = textureImageView;
    
    VkImage dstImage = NULL;
    /*
    vkGetSwapchainImagesKHR(gf3d_swapchain.device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount, NULL);
    if (gf3d_swapchain.swapImageCount == 0)
    {
        slog("failed to create any swap images!");
        gf3d_swapchain_close();
        return;
    }
    gf3d_swapchain.swapImages = (VkImage *)gfc_allocate_array(sizeof(VkImage),gf3d_swapchain.swapImageCount);*/
    //vkGetSwapchainImagesKHR(gf3d_swapchain.device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount,gf3d_swapchain.swapImages );
    //slog("created swap chain with %i images",gf3d_swapchain.swapImageCount);
    
    
    //vSwapChain swap = gf3d_swapchain_get_images(index);
//     if(index > gf3d_swapchain.swapImageCount){
//      dstImage = gf3d_swapchain.swapImages[0];  
//     }
//     else{
     
     
     
     dstImage = gf3d_swapchain.swapImages[i];
    
    //dstImage = gf3d_swapchain_get_frame_buffer_by_index(index)->image;
     
     
     //}
    //dstImage = gf3d_swapchain.depthImage;
    
    if(dstImage == NULL){
        slog("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
    }
    
    VkExtent2D dstextent = gf3d_swapchain_get_extent();
    
    //texture.TextureImage;
    
    //slog("dstImage file %s",dstImage);
    
    //slog("dstImage width %i",dstextent.width);
   // slog("dstImage height %i",dstextent.height);
    
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

    /*   
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
        */
        
        int mipWidth = dstextent.width ;
        int mipHeight = dstextent.height;
        //int mipLevels = 1;
        int Width = 1024 ;
        int Height = 1024 ;
       /* 
        copyCmd,
			srcImage,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
        */
       
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
            barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier2.srcAccessMask = 0;
            barrier2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier2);
            
           
            
             VkImageBlit blit = {0};
             
     
             blit.srcOffsets[0].x = srcWidthStart;//0
             blit.srcOffsets[0].y = srcHeightStart;//0
             blit.srcOffsets[0].z = 0;
         blit.srcOffsets[1].x = srcWidthEnd;
         blit.srcOffsets[1].y = srcHeightEnd;
             //blit.srcOffsets[1].x = 32;
            // blit.srcOffsets[1].y = 32;
             blit.srcOffsets[1].z = 1;
             blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
             
      /*       
    blit.srcSubresource.baseMipLevel = 0;
    blit.srcSubresource.levelCount = 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
             */
             
      Entity *player;
        player = player_get();
      
      
             blit.srcSubresource.mipLevel = 0;
             blit.srcSubresource.baseArrayLayer = 0;
             blit.srcSubresource.layerCount = 1;
             blit.dstOffsets[0].x = dstWidthStart;//0
             blit.dstOffsets[0].y = dstHeightStart;//0
             blit.dstOffsets[0].z = 0;
        
             //blit.dstOffsets[1].x = player->health;//500
             //blit.dstOffsets[1].y = player->health;//200
             
             blit.dstOffsets[1].x = dstWidthEnd;//500
             blit.dstOffsets[1].y = dstHeightEnd;//200
             
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
            //slog("blit");
            
    /*        
             // Use a barrier to make sure the blit is finished before the copy starts
    VkImageMemoryBarrier memBarrier = {0};
    memBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memBarrier.pNext = NULL;
    memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    memBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    memBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    memBarrier.subresourceRange.baseMipLevel = 0;
    memBarrier.subresourceRange.levelCount = 1;
    memBarrier.subresourceRange.baseArrayLayer = 0;
    memBarrier.subresourceRange.layerCount = 1;
    memBarrier.image = dstImage;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &memBarrier);

    // Do a image copy to part of the dst image - checks should stay small
    VkImageCopy cregion;
    cregion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cregion.srcSubresource.mipLevel = 0;
    cregion.srcSubresource.baseArrayLayer = 0;
    cregion.srcSubresource.layerCount = 1;
    cregion.srcOffset.x = 0;
    cregion.srcOffset.y = 0;
    cregion.srcOffset.z = 0;
    cregion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cregion.dstSubresource.mipLevel = 0;
    cregion.dstSubresource.baseArrayLayer = 0;
    cregion.dstSubresource.layerCount = 1;
    cregion.dstOffset.x = 256;
    cregion.dstOffset.y = 256;
    cregion.dstOffset.z = 0;
    cregion.extent.width = 128;
    cregion.extent.height = 128;
    cregion.extent.depth = 1;

    vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &cregion);
            
            */
            
            
            
            
            
            
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
            barrier2.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier2.dstAccessMask = 0;
        
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier2);

            
            //gf3d_swapchain.imageViews = (VkImageView *)gfc_allocate_array(sizeof(VkImageView),gf3d_swapchain.swapImageCount);
    
    
   }   
            
//             gf3d_swapchain.imageViews[index] = gf3d_vgraphics_create_image_view(dstImage,gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format);
            
}
void gf3d_swapchain_blit_health(VkCommandBuffer commandBuffer,Uint32 srcWidthStart,Uint32 srcWidthEnd,Uint32 srcHeightStart,Uint32 srcHeightEnd,Uint32 dstWidthStart,Uint32 dstWidthEnd,Uint32 dstHeightStart,Uint32 dstHeightEnd,Texture *texture){

    VkImageSubresourceRange ImageSubresourceRange;
   ImageSubresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   ImageSubresourceRange.baseMipLevel   = 0;
   ImageSubresourceRange.levelCount     = 1;
   ImageSubresourceRange.baseArrayLayer = 0;
   ImageSubresourceRange.layerCount     = 1;

   VkClearColorValue ClearColorValue = {{ 1.0, 0.0, 0.0, 0.0 }};

   for (int i = 0; i < gf3d_swapchain.swapImageCount; i++)
   {
      //VkCommandBuffer CommandBuffer = CommandBuffers[i];

      //Result = vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);

      //PrintVkResult("vkBeginCommandBuffer", Result);

      //vkCmdClearColorImage(commandBuffer, gf3d_swapchain.swapImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColorValue, 1, &ImageSubresourceRange);

     // Result = vkEndCommandBuffer(CommandBuffer);

      //PrintVkResult("vkEndCommandBuffer", Result);
   //}
    
    
    
    //Texture *texture = gf3d_texture_load("images/dino.png");
    
    VkImage srcImage = texture->textureImage;
    
    
//     for (int i = 0; i < gf3d_swapchain.swapImageCount; i++)
//    {
//       //VkCommandBuffer CommandBuffer = CommandBuffers[i];
// 
//       //Result = vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);
// 
//       //PrintVkResult("vkBeginCommandBuffer", Result);
// 
        vkCmdClearColorImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColorValue, 1, &ImageSubresourceRange);
// 
//       //Result = vkEndCommandBuffer(CommandBuffer);
// 
//       //PrintVkResult("vkEndCommandBuffer", Result);
//    }
//     
   // vkCmdClearColorImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ClearColorValue, 1, &ImageSubresourceRange);
    
    
    //VkExtent2D dstextent = textureImageView;
    
    VkImage dstImage = NULL;
    /*
    vkGetSwapchainImagesKHR(gf3d_swapchain.device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount, NULL);
    if (gf3d_swapchain.swapImageCount == 0)
    {
        slog("failed to create any swap images!");
        gf3d_swapchain_close();
        return;
    }
    gf3d_swapchain.swapImages = (VkImage *)gfc_allocate_array(sizeof(VkImage),gf3d_swapchain.swapImageCount);*/
    //vkGetSwapchainImagesKHR(gf3d_swapchain.device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount,gf3d_swapchain.swapImages );
    //slog("created swap chain with %i images",gf3d_swapchain.swapImageCount);
    
    
    //vSwapChain swap = gf3d_swapchain_get_images(index);
//     if(index > gf3d_swapchain.swapImageCount){
//      dstImage = gf3d_swapchain.swapImages[0];  
//     }
//     else{
     
     
     
     dstImage = gf3d_swapchain.swapImages[i];
    
    //dstImage = gf3d_swapchain_get_frame_buffer_by_index(index)->image;
     
     
     //}
    //dstImage = gf3d_swapchain.depthImage;
    
    if(dstImage == NULL){
        slog("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
    }
    
    VkExtent2D dstextent = gf3d_swapchain_get_extent();
    
    //texture.TextureImage;
    
    //slog("dstImage file %s",dstImage);
    
    //slog("dstImage width %i",dstextent.width);
   // slog("dstImage height %i",dstextent.height);
    
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

    /*   
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
        */
        
        int mipWidth = dstextent.width ;
        int mipHeight = dstextent.height;
        //int mipLevels = 1;
        int Width = 1024 ;
        int Height = 1024 ;
       /* 
        copyCmd,
			srcImage,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
        */
       
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
            barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier2.srcAccessMask = 0;
            barrier2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier2);
            
           
            
             VkImageBlit blit = {0};
             
     
             blit.srcOffsets[0].x = srcWidthStart;//0
             blit.srcOffsets[0].y = srcHeightStart;//0
             blit.srcOffsets[0].z = 0;
         blit.srcOffsets[1].x = srcWidthEnd;
         blit.srcOffsets[1].y = srcHeightEnd;
             //blit.srcOffsets[1].x = 32;
            // blit.srcOffsets[1].y = 32;
             blit.srcOffsets[1].z = 1;
             blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
             
      /*       
    blit.srcSubresource.baseMipLevel = 0;
    blit.srcSubresource.levelCount = 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
             */
             
      Entity *player;
    player = player_get();
      if(player != NULL){
      
             blit.srcSubresource.mipLevel = 0;
             blit.srcSubresource.baseArrayLayer = 0;
             blit.srcSubresource.layerCount = 1;
             blit.dstOffsets[0].x = dstWidthStart;//0
             blit.dstOffsets[0].y = dstHeightStart;//0
             blit.dstOffsets[0].z = 0;
        
             //blit.dstOffsets[1].x = player->health;//500
             //blit.dstOffsets[1].y = player->health;//200
             
             blit.dstOffsets[1].x = player->health;//500
             blit.dstOffsets[1].y = player->health;//200
             
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
            //slog("blit");
            
            
            
            
            
            
            
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
            barrier2.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier2.dstAccessMask = 0;
        
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier2);

      }
      else{
          blit.srcSubresource.mipLevel = 0;
             blit.srcSubresource.baseArrayLayer = 0;
             blit.srcSubresource.layerCount = 1;
             blit.dstOffsets[0].x = dstWidthStart;//0
             blit.dstOffsets[0].y = dstHeightStart;//0
             blit.dstOffsets[0].z = 0;
        
             //blit.dstOffsets[1].x = player->health;//500
             //blit.dstOffsets[1].y = player->health;//200
             
             blit.dstOffsets[1].x = 1;//500
             blit.dstOffsets[1].y = 1;//200
             
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
            //slog("blit");
            
            
            
            
            
            
            
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
            barrier2.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier2.dstAccessMask = 0;
        
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                0, NULL,
                0, NULL,
                1, &barrier2);
          
      }
            //gf3d_swapchain.imageViews = (VkImageView *)gfc_allocate_array(sizeof(VkImageView),gf3d_swapchain.swapImageCount);
    
    
   }   
            
//             gf3d_swapchain.imageViews[index] = gf3d_vgraphics_create_image_view(dstImage,gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format);
            
}

/*eol@eof*/
