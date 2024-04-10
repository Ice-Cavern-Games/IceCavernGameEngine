#include <descriptors.h>

#include <swap_chain.h>

namespace IC::Renderer
{
    void DescriptorLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType type)
    {
        VkDescriptorSetLayoutBinding newBinding{};
        newBinding.binding = binding;
        newBinding.descriptorCount = 1;
        newBinding.descriptorType = type;

        bindings.push_back(newBinding);
    }

    void DescriptorLayoutBuilder::clear()
    {
        bindings.clear();
    }

    VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages)
    {
        for (VkDescriptorSetLayoutBinding &binding : bindings)
        {
            binding.stageFlags |= shaderStages;
        }

        VkDescriptorSetLayoutCreateInfo info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        info.pNext = nullptr;

        info.pBindings = bindings.data();
        info.bindingCount = (uint32_t)bindings.size();
        info.flags = 0;

        VkDescriptorSetLayout set;
        VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

        return set;
    }

    void DescriptorWriter::write_image(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
    {
        VkDescriptorImageInfo info = imageInfos.emplace_back(VkDescriptorImageInfo{
            .sampler = sampler,
            .imageView = image,
            .imageLayout = layout});

        VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

        write.dstBinding = binding;
        write.dstSet = VK_NULL_HANDLE;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = &info;

        writes.push_back(write);
    }

    void DescriptorWriter::write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
    {
        VkDescriptorBufferInfo &info = bufferInfos.emplace_back(VkDescriptorBufferInfo{
            .buffer = buffer,
            .offset = offset,
            .range = size});

        VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

        write.dstBinding = binding;
        write.dstSet = VK_NULL_HANDLE;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = &info;

        writes.push_back(write);
    }

    void DescriptorWriter::clear()
    {
        imageInfos.clear();
        writes.clear();
        bufferInfos.clear();
    }

    void DescriptorWriter::update_set(VkDevice device, VkDescriptorSet set)
    {
        for (VkWriteDescriptorSet &write : writes)
        {
            write.dstSet = set;
        }

        vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
    }

    void DescriptorAllocator::createDescriptorPool(VkDevice device, std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxSets)
    {
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;

        VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
    }

    void DescriptorAllocator::allocateDescriptorSets(VkDevice device, VkDescriptorSetLayout layout, std::vector<VkDescriptorSet> &descriptorSets)
    {
        std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()));
    }

    void DescriptorAllocator::destroyDescriptorPool(VkDevice device)
    {
        vkDestroyDescriptorPool(device, pool, nullptr);
    }
}