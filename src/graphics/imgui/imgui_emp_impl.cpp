#include "imgui_emp_impl.hpp"
#include <iterator>
#include "debug/debug.hpp"
#include "graphics/renderer.hpp"

namespace emp {

// Your Vulkan initialization, ImGui setup, and rendering loop

VkDescriptorPool ImGuiGetDescriptorPool(VkDevice device) {
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	auto result = vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create a descriptor pool for imgui!");
    }
    return imguiPool;
}
//TODO fix ugly
ImGui_ImplVulkan_InitInfo last_init_info;
void ImGuiSetup(
        ImGui_ImplVulkan_InitInfo init_info,
        GLFWwindow* window,
        Device& device,
        Renderer& rend,
        VkRenderPass renderPass
) {
    last_init_info = init_info;
    //1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.


	// 2: initialize imgui library

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(window, true);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_Init(&init_info, renderPass);

	//execute a gpu command to upload imgui font textures
    auto command_buffer = device.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    device.endSingleTimeCommands(command_buffer);

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiRenderBegin() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void ImGuiRenderEnd(VkCommandBuffer commandBuffer) {
    // Render ImGui frame
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
void ImGuiDestroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(last_init_info.Device, last_init_info.DescriptorPool, nullptr);
}

};
