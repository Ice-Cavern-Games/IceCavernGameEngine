#pragma once

#include "ic_renderer.h"

#include "descriptors.h"
#include "pipelines.h"
#include "swap_chain.h"
#include "vulkan_types.h"

namespace IC {
    class VulkanRenderer : public Renderer {
    public:
        VulkanRenderer(RendererConfig &config);
        ~VulkanRenderer();

        void DrawFrame();
        void AddMesh(Mesh &meshData, Material &materialData);

    private:
        void CreateCommandBuffers();
        void InitDescriptorAllocator();

        VulkanDevice _vulkanDevice{window};
        SwapChain _swapChain;
        PipelineManager _pipelineManager{};
        DescriptorAllocator _descriptorAllocator{};

        std::vector<MeshRenderData> _renderData;
        std::vector<VkCommandBuffer> _cBuffers;
    };
} // namespace IC
