#include "vulkan_initializers.h"

#include <ic_log.h>

#include "descriptors.h"
#include "pipelines.h"
#include "swap_chain.h"
#include "vulkan_util.h"

#include <iostream>

namespace IC {
    VmaAllocatorCreateInfo AllocatorCreateInfo(VulkanDevice &device) {
        VmaAllocatorCreateInfo info = {};

        info.flags = 0;
        info.vulkanApiVersion = VULKAN_API_VERSION;
        info.physicalDevice = device.PhysicalDevice();
        info.device = device.Device();
        info.instance = device.Instance();

        return info;
    }

    VkRenderingAttachmentInfo AttachmentInfo(VkImageView view, VkClearValue *clear, VkImageLayout layout) {
        VkRenderingAttachmentInfo info = {.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
        info.pNext = nullptr;

        info.imageView = view;
        info.imageLayout = layout;
        info.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        if (clear) {
            info.clearValue = *clear;
        }

        return info;
    }

    VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/) {
        VkCommandBufferBeginInfo info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        info.pNext = nullptr;
        info.pInheritanceInfo = nullptr;
        info.flags = flags;

        return info;
    }

    VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd) {
        VkCommandBufferSubmitInfo info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO};
        info.pNext = nullptr;
        info.commandBuffer = cmd;
        info.deviceMask = 0;

        return info;
    }

    VkImageCreateInfo ImageCreateInfo(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                      VkImageUsageFlags usage) {
        VkImageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.extent.width = static_cast<uint32_t>(width);
        info.extent.height = static_cast<uint32_t>(height);
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.format = format;
        info.tiling = tiling;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = usage;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.flags = 0;

        return info;
    }

    VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspect) {
        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = format;
        info.subresourceRange.aspectMask = aspect;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        return info;
    }

    VkRenderingInfo RenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo *colorAttachment,
                                  VkRenderingAttachmentInfo *depthAttachment) {
        VkRenderingInfo info{.sType = VK_STRUCTURE_TYPE_RENDERING_INFO};
        info.pNext = nullptr;
        info.renderArea = VkRect2D{VkOffset2D{0, 0}, renderExtent};
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = colorAttachment;
        info.pDepthAttachment = depthAttachment;
        info.pStencilAttachment = nullptr;

        return info;
    }

    VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo *cmd, VkSemaphoreSubmitInfo *signalSemaphoreInfo,
                             VkSemaphoreSubmitInfo *waitSemaphoreInfo) {
        VkSubmitInfo2 info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
        info.pNext = nullptr;
        info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
        info.pWaitSemaphoreInfos = waitSemaphoreInfo;
        info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
        info.pSignalSemaphoreInfos = signalSemaphoreInfo;
        info.commandBufferInfoCount = 1;
        info.pCommandBufferInfos = cmd;

        return info;
    }

    // descriptors
    void WritePerObjectDescriptors(VulkanAllocator &allocator, SwapChain &swapChain, DescriptorWriter &writer,
                                   MeshRenderData &renderData) {
        size_t maxFrames = SwapChain::MAX_FRAMES_IN_FLIGHT;
        renderData.mvpBuffers.resize(maxFrames);

        for (size_t i = 0; i < maxFrames; i++) {
            // hard coded for now
            CameraDescriptors projection{};
            projection.proj = glm::perspective(
                glm::radians(45.0f),
                (float)swapChain.GetSwapChainExtent().width / swapChain.GetSwapChainExtent().height, 0.1f, 10.0f);

            allocator.CreateBuffer(sizeof(CameraDescriptors), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO,
                                   renderData.mvpBuffers[i]);

            memcpy(renderData.mvpBuffers[i].allocInfo.pMappedData, &projection, sizeof(CameraDescriptors));

            writer.WriteBuffer(0, renderData.mvpBuffers[i].buffer, sizeof(CameraDescriptors), 0,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }
    }

    void WriteLightDescriptors(VulkanAllocator &allocator, size_t maxFrames, SceneLightDescriptors &lightData,
                               DescriptorWriter &writer, std::vector<AllocatedBuffer> &lightBuffers) {
        lightBuffers.resize(maxFrames);
        for (size_t i = 0; i < maxFrames; i++) {
            allocator.CreateBuffer(sizeof(SceneLightDescriptors), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VMA_MEMORY_USAGE_AUTO, lightBuffers[i]);

            memcpy(lightBuffers[i].allocInfo.pMappedData, &lightData, sizeof(SceneLightDescriptors));
            writer.WriteBuffer(0, lightBuffers[i].buffer, sizeof(SceneLightDescriptors), 0,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }
    }

    void WriteMaterialDescriptors(VulkanAllocator &allocator, size_t maxFrames, DescriptorWriter &writer,
                                  MaterialInstance &material, VulkanTextureManager &textureManager,
                                  std::vector<AllocatedBuffer> &materialBuffers) {
        // create material buffer
        materialBuffers.resize(maxFrames);
        VkDeviceSize size = 0;
        std::map<int, size_t> offsets;
        for (auto &[index, binding] : material.BindingValues()) {
            offsets[index] = size;

            // uniform binding types only
            if (binding.binding->bindingType == BindingType::Uniform) {
                size += static_cast<VkDeviceSize>(binding.size);
            }
        }

        for (size_t i = 0; i < maxFrames; i++) {
            allocator.CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, materialBuffers[i]);

            for (auto &[index, binding] : material.BindingValues()) {
                if (binding.binding->bindingType == BindingType::Texture) {
                    AllocatedImage *texture = textureManager.GetTexture(*static_cast<std::string *>(binding.value));
                    writer.WriteImage(index, texture->view, textureManager.DefaultSampler(),
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                } else {
                    writer.WriteBuffer(index, materialBuffers[i].buffer, binding.size, offsets[index],
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
                }
            }
        }
    }

    DirectionalLightDescriptors CreateDirectionalLightDescriptors(DirectionalLight *directionalLight,
                                                                  glm::mat4 viewMat) {
        glm::vec3 directionalViewSpaceDirection = viewMat * glm::vec4(directionalLight->Direction(), 0.0f);

        DirectionalLightDescriptors descriptors{};
        descriptors.dir = directionalViewSpaceDirection;
        descriptors.diff = directionalLight->Color();
        descriptors.amb = directionalLight->Ambient();
        descriptors.spec = directionalLight->Specular();

        return descriptors;
    }

    PointLightDescriptors CreatePointLightDescriptors(PointLight *pointLight, glm::vec3 lightPosition,
                                                      glm::mat4 viewMat) {
        glm::vec3 lightViewSpacePos = viewMat * glm::vec4(lightPosition, 1.0f);

        PointLightDescriptors descriptors{};
        descriptors.pos = lightViewSpacePos;
        descriptors.diff = pointLight->Color();
        descriptors.amb = pointLight->Ambient();
        descriptors.spec = pointLight->Specular();
        descriptors.cons = pointLight->Constant();
        descriptors.lin = pointLight->Linear();
        descriptors.quad = pointLight->Quadratic();

        return descriptors;
    }

    // images
    void CreateImageSampler(VkDevice device, float maxAnisotropy, VkSampler &textureSampler) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = maxAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            IC_CORE_ERROR("Failed to create texture sampler.");
        }
    }

    // pipelines
    std::shared_ptr<Pipeline> CreateOpaquePipeline(VkDevice device, SwapChain &swapChain,
                                                   MaterialInstance &materialData) {
        std::vector<VkDescriptorSetLayout> descriptorSets;
        // per object descriptors
        DescriptorLayoutBuilder descriptorLayoutBuilder{};
        descriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // Camera Descriptors
        descriptorSets.push_back(descriptorLayoutBuilder.Build(device, VK_SHADER_STAGE_VERTEX_BIT));
        descriptorLayoutBuilder.Clear();

        // lit descriptors
        if (materialData.Template().flags & MaterialFlags::Lit) {
            descriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // scene light data
            descriptorSets.push_back(descriptorLayoutBuilder.Build(device, VK_SHADER_STAGE_FRAGMENT_BIT));
            descriptorLayoutBuilder.Clear();
        }

        // material descriptors
        for (auto &[index, value] : materialData.BindingValues()) {
            descriptorLayoutBuilder.AddBinding(index, value.binding->bindingType == BindingType::Texture
                                                          ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                                                          : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }
        if (materialData.BindingValues().size() > 0) {
            descriptorSets.push_back(
                descriptorLayoutBuilder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));
        }

        // pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSets.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSets.data();

        // push constants
        VkPushConstantRange pushConstants =
            PushConstants<TransformationPushConstants>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstants;

        VkPipelineLayout pipelineLayout;
        VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        PipelineBuilder pipelineBuilder;

        pipelineBuilder.pipelineLayout = pipelineLayout;
        VkShaderModule vertShaderModule =
            PipelineBuilder::CreateShaderModule(device, materialData.Template().vertShaderData);
        VkShaderModule fragShaderModule =
            PipelineBuilder::CreateShaderModule(device, materialData.Template().fragShaderData);

        pipelineBuilder.SetShaders(vertShaderModule, fragShaderModule);
        pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
        pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
        pipelineBuilder.SetMultisamplingNone();
        materialData.Template().flags &MaterialFlags::Transparent ? pipelineBuilder.EnableBlending()
                                                                  : pipelineBuilder.DisableBlending();
        pipelineBuilder.EnableDepthTest();
        pipelineBuilder.SetColorAttachmentFormat(swapChain.GetSwapChainImageFormat());
        pipelineBuilder.SetDepthFormat(swapChain.GetSwapChainDepthFormat());

        std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>();
        pipeline->pipeline = pipelineBuilder.BuildPipeline(device);
        pipeline->layout = pipelineLayout;
        pipeline->descriptorSetLayouts = descriptorSets;
        pipeline->shaderModules = {vertShaderModule, fragShaderModule};
        pipeline->materialFlags = materialData.Template().flags;

        return pipeline;
    }

    // ImGui
    void InitImGui(VulkanDevice &device, GLFWwindow *window, VkDescriptorPool descriptorPool, VkFormat imageFormat) {
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = device.Instance();
        initInfo.PhysicalDevice = device.PhysicalDevice();
        initInfo.Device = device.Device();
        initInfo.Queue = device.GraphicsQueue();
        initInfo.DescriptorPool = descriptorPool;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.UseDynamicRendering = true;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.ColorAttachmentFormat = imageFormat;

        ImGui_ImplVulkan_Init(&initInfo, VK_NULL_HANDLE);
    }
} // namespace IC
