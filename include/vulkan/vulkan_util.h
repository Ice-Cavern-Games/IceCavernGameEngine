#pragma once

#include <vulkan_types.h>
#include <vulkan_device.h>

namespace IC::Renderer::Util
{
    void CreateAndFillBuffer(VulkanDevice &device, const void *srcData, VkDeviceSize bufferSize, VkBufferUsageFlags buffer_usage_flags, VkMemoryPropertyFlags memory_property_flags, AllocatedBuffer &allocatedBuffer);
    VkDescriptorType MaterialInputTypeMapping(MaterialInputType inputType);

    void CopyImageToImage(VkCommandBuffer commandBuffer, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
    void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    // void load_texture_image(VulkanDevice &device, std::string texturePath, AllocatedImage &outImage);
}