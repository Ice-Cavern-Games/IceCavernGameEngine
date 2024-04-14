#pragma once

#include "ic_renderer.h"

#include "descriptors.h"
#include "pipelines.h"
#include "swap_chain.h"
#include "vulkan_initializers.h"
#include "vulkan_types.h"

namespace IC {
    class VulkanRenderer : public Renderer {
    public:
        VulkanRenderer(RendererConfig &config);
        ~VulkanRenderer();

        void AddMesh(Mesh &meshData, Material &materialData);
        void DrawFrame();

    private:
        void CreateCommandBuffers();
        void InitDescriptorAllocators();
        void RenderImGui(VkCommandBuffer cBuffer, VkImageView targetImageView);
        PFN_vkCmdBeginRenderingKHR VulkanBeginRendering;
        PFN_vkCmdEndRendering VulkanEndRendering;

        VulkanDevice _vulkanDevice{window};
        SwapChain _swapChain;
        PipelineManager _pipelineManager{};
        DescriptorAllocator _meshDescriptorAllocator{};
        DescriptorAllocator _imGuiDescriptorAllocator{};

        std::vector<MeshRenderData> _renderData;
        std::vector<VkCommandBuffer> _cBuffers;

        // render functions for mac support
    };
} // namespace IC
