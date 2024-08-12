#ifndef EMP_RENDERER_HPP
#define EMP_RENDERER_HPP

#include "vulkan/device.hpp"
#include "vulkan/swap_chain.hpp"
#include "io/window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace emp {
    class Renderer {
    public:
        Renderer(Window &window, Device &device);

        ~Renderer();

        Renderer(const Renderer &) = delete;

        Renderer &operator=(const Renderer &) = delete;

        [[nodiscard]] VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }

        [[nodiscard]] float getAspectRatio() const { return swapChain->extentAspectRatio(); }

        [[nodiscard]] bool isFrameInProgress() const { return isFrameStarted; }

        [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        [[nodiscard]] int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();

        void endFrame();

        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);

        void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

    private:
        void createCommandBuffers();

        void freeCommandBuffers();

        void recreateSwapChain();

        Window &window;
        Device &device;
        std::unique_ptr<SwapChain> swapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex{};
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };
}  // namespace emp

#endif
