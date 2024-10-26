#include "core/layer.hpp"
#include "graphics/animated_sprite.hpp"
#include "scene/app.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

// Load SPIR-V shader code from file
std::vector<char> readShaderFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    return buffer;
}

// Helper function to find suitable memory type for buffer allocation
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type.");
}

using namespace emp;

enum CollisionLayers {
    GROUND,
    PLAYER,
    ITEM
};
class Demo : public App {
    public:
        Entity mouse_entity;
        Entity protagonist;

        
        float protagonist_speed = 600.f;
        Entity isProtagonistGrounded;
        float isProtagonistGroundedSec;
        static constexpr float cayote_time = 0.25f;
        static constexpr float cube_side_len = 75.f;
        std::vector<vec2f> unit_cube = {
                vec2f(-1.f/2.f, -1.f/2.f),
                vec2f(-1.f/2.f, 1.f/2.f),
                vec2f(1.f/2.f, 1.f/2.f),
                vec2f(1.f/2.f, -1.f/2.f)
        };
        std::vector<vec2f> cube_model_shape = {
                vec2f(-cube_side_len/2, -cube_side_len/2),
                vec2f(-cube_side_len/2, cube_side_len/2),
                vec2f(cube_side_len/2, cube_side_len/2),
                vec2f(cube_side_len/2, -cube_side_len/2)
        };
        static constexpr int markers_count = 64;
        Entity markers[markers_count];
        Entity last_created_crate = 0;
        DebugShape debug_cube_shape = DebugShape(
                device,
                cube_model_shape,
                glm::vec4(1, 0, 0, 1),
                glm::vec4(0, 0, 1, 1)
        );
        void setupAnimationForProtagonist();
        void onRender(Device&, const FrameInfo& frame) override final;
        void onSetup(Window& window, Device& device) override final;
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final;
        void onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller)override final;

        Demo()
            : App({{"../assets/models/colored_cube.obj", "cube"}},
                {
                    {"../assets/textures/dummy.png", "dummy"},
                    {"../assets/textures/crate.jpg", "crate"},
                    {"../assets/textures/knight/_Run.png", "running"},
                    {"../assets/textures/knight/_Jump.png", "jump-up"},
                    {"../assets/textures/knight/_Fall.png", "jump-down"},
                    {"../assets/textures/knight/_Idle.png", "idle"},
                    {"../assets/textures/knight/_JumpFallInbetween.png", "jumpfall"}
                }) {}
};
void Demo::onSetup(Window& window, Device& device) {
    // {
    //     auto& tex = Texture("invalid").texture();
    //     auto pixels = tex.getPixelsFromGPU();
    //     auto h = tex.getExtent().height;
    //     auto w = tex.getExtent().width;
    //     for(int i = 0; i < h; i += 1) {
    //         for(int ii = 0; ii < w; ii += 1) {
    //             auto r = (int)(pixels)[i * w + ii].red;
    //             auto g = (int)(pixels)[i * w + ii].green;
    //             auto b = (int)(pixels)[i * w + ii].blue;
    //             std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
    //             std::cout << "\u2588";
    //         }
    //         std::cout << '\n';
    //     }
    //     std::cout << "\033[0m";
    // }
    // controller.bind(eKeyMappings::Ability1, GLFW_KEY_C);
    // controller.bind(eKeyMappings::Jump, GLFW_KEY_SPACE);
    //
    // controller.bind(eKeyMappings::LookUp, GLFW_KEY_W);
    // controller.bind(eKeyMappings::LookDown, GLFW_KEY_S);
    // controller.bind(eKeyMappings::LookLeft, GLFW_KEY_D);
    // controller.bind(eKeyMappings::LookRight, GLFW_KEY_A);
    //
    // controller.bind(eKeyMappings::MoveUp, GLFW_KEY_UP);
    // controller.bind(eKeyMappings::MoveDown, GLFW_KEY_DOWN);
    // controller.bind(eKeyMappings::MoveLeft, GLFW_KEY_LEFT);
    // controller.bind(eKeyMappings::MoveRight, GLFW_KEY_RIGHT);
    //
    // // coordinator.addComponent(cube, Model("cube"));
    // protagonist = coordinator.createEntity();
    //
    // coordinator.getSystem<ColliderSystem>()
    //     ->onCollisionEnter(protagonist,
    //         protagonist,
    //         [&](const CollisionInfo& info) {
    //             auto ang = angle(info.collision_normal, vec2f(0, 1));
    //             if (cos(ang) > -M_PI / 4.f) {
    //                 return;
    //             }
    //             isProtagonistGrounded = info.collidee_entity;
    //             EMP_LOG_DEBUG << "grounded: " << isProtagonistGrounded;
    //         })
    //     .onCollisionExit(
    //         protagonist, protagonist, [&](Entity me, Entity other) {
    //             if (other == isProtagonistGrounded || isProtagonistGrounded == false) {
    //                 isProtagonistGrounded = false;
    //                 EMP_LOG_DEBUG << "ungrounded";
    //             }
    //         });
    //
    // mouse_entity = coordinator.createEntity();
    // coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));
    //
    // {
    //     Rigidbody rb;
    //     rb.useAutomaticMass = true;
    //     rb.isRotationLocked = true;
    //     coordinator.addComponent(protagonist, Transform(vec2f(), 0.f));
    //     std::vector<vec2f> protagonist_shape = {
    //             vec2f(-100.f/4, -100.f/1.5),
    //             vec2f(-100.f/4, 100.f/1.5),
    //             vec2f(100.f/4, 100.f/1.5),
    //             vec2f(100.f/4, -100.f/1.5)
    //     };
    //     auto col = Collider(protagonist_shape);
    //     col.collider_layer = PLAYER;
    //
    //     coordinator.addComponent(protagonist, col);
    //     coordinator.addComponent(protagonist, rb);
    //
    //     auto mat = Material(); 
    //     coordinator.addComponent(protagonist, mat);
    //
    //     auto db_shape = DebugShape(device, protagonist_shape, glm::vec4(1, 1, 1, 1));
    //     coordinator.addComponent(protagonist, db_shape);
    //
    //
    //     setupAnimationForProtagonist();
    // }
    //
    // std::pair<vec2f, float> ops[4] = {
    //         {vec2f(0.f, 400.0f), 0.f},
    //         {vec2f(400.f, 0.0f), M_PI / 2.f},
    //         {vec2f(0.f, -400.0f), 0.f},
    //         {vec2f(-400.f, 0.0f), M_PI / 2.f}
    // };
    // debug_cube_shape.fill_color = glm::vec4(1, 1, 1, 1);
    // for (auto o : ops) {
    //     auto platform = coordinator.createEntity();
    //     coordinator.addComponent(
    //             platform, Transform(o.first, o.second, {width / cube_side_len, 1.f})
    //     );
    //     coordinator.addComponent(
    //             platform, DebugShape(device, cube_model_shape)
    //     );
    //     auto col = Collider(cube_model_shape);
    //     col.collider_layer = GROUND;
    //     coordinator.addComponent(platform, col);
    //     coordinator.addComponent(platform, Rigidbody{true});
    //     coordinator.addComponent(platform, Material());
    // }
    // for (auto& marker : markers) {
    //     marker = coordinator.createEntity();
    //     coordinator.addComponent(marker, Transform(vec2f(INFINITY, INFINITY), 0.f, {0.1f, 0.1f}));
    //     coordinator.addComponent(marker, DebugShape(device, unit_cube, glm::vec4(0.8, 0.8, 1, 1)));
    // }
    // debug_cube_shape.fill_color = glm::vec4(1, 1, 0, 1);
    //
    // coordinator.getSystem<PhysicsSystem>()->gravity = 1000.f;
    // coordinator.getSystem<PhysicsSystem>()->substep_count = 8;
    const size_t dataCount = 1024;
    VkDeviceSize bufferSize = sizeof(float) * dataCount;
    std::vector<float> inputData(dataCount, 2.0f);  // Sample input data: all values set to 2.0

    // Create buffer
    VkBuffer dataBuffer;
    VkDeviceMemory dataBufferMemory;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(device.device(), &bufferInfo, nullptr, &dataBuffer);

    // Allocate memory for buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.device(), dataBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, 
                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(device.device(), &allocInfo, nullptr, &dataBufferMemory);
    vkBindBufferMemory(device.device(), dataBuffer, dataBufferMemory, 0);

    // Map memory and upload data
    void* mappedMemory;
    vkMapMemory(device.device(), dataBufferMemory, 0, bufferSize, 0, &mappedMemory);
    memcpy(mappedMemory, inputData.data(), bufferSize);
    vkUnmapMemory(device.device(), dataBufferMemory);

    // Load compute shader
    auto shaderCode = readShaderFile("../assets/shaders/compute.comp.spv");

    VkShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleInfo.codeSize = shaderCode.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule computeShaderModule;
    vkCreateShaderModule(device.device(), &shaderModuleInfo, nullptr, &computeShaderModule);

    // Create descriptor set layout
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorSetLayout);

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipelineLayout;
    vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

    // Create compute pipeline
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = computeShaderModule;
    shaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = pipelineLayout;

    VkPipeline computePipeline;
    vkCreateComputePipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline);

    // Descriptor pool and set allocation
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool);

    VkDescriptorSetAllocateInfo allocInfoDS{};
    allocInfoDS.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfoDS.descriptorPool = descriptorPool;
    allocInfoDS.descriptorSetCount = 1;
    allocInfoDS.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet descriptorSet;
    vkAllocateDescriptorSets(device.device(), &allocInfoDS, &descriptorSet);

    VkDescriptorBufferInfo bufferInfoDS{};
    bufferInfoDS.buffer = dataBuffer;
    bufferInfoDS.offset = 0;
    bufferInfoDS.range = bufferSize;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfoDS;

    vkUpdateDescriptorSets(device.device(), 1, &descriptorWrite, 0, nullptr);

    auto commandBuffer = device.beginSingleTimeCommands();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdDispatch(commandBuffer, (dataCount + 63) / 64, 1, 1); // Dispatch work

    // End, submit and retrieve data
    device.endSingleTimeCommands(commandBuffer);

    // Map memory again to read back data
    vkMapMemory(device.device(), dataBufferMemory, 0, bufferSize, 0, &mappedMemory);
    float* resultData = reinterpret_cast<float*>(mappedMemory);
    EMP_LOG_DEBUG << "!";
    for (size_t i = 0; i < dataCount; ++i) {
        std::cout << "Result " << i << ": " << resultData[i] << std::endl;
    }
    vkUnmapMemory(device.device(), dataBufferMemory);
    exit(1);

    // Cleanup omitted for brevity
}

void Demo::setupAnimationForProtagonist() {
    auto def_size = vec2f(300.f, 300.f);
    auto offset = vec2f(10.f, -def_size.y / 3.5f);
    MovingSprite idle_moving;
    {
        Sprite idle_sprite = Sprite(Texture("idle"), def_size);
        idle_sprite.position_offset = offset;
        idle_sprite.centered = true;
        idle_sprite.hframes = 10;
        idle_sprite.vframes = 1;
        idle_moving.sprite = idle_sprite;
        for(int i = 0; i < idle_sprite.frameCount(); i++) {
            idle_moving.add(i, 0.085f);
        }
    }
    AnimatedSprite::Builder build("idle", idle_moving);
    {
        {
            Sprite running_sprite = Sprite(Texture("running"), def_size);
            running_sprite.position_offset = offset;
            running_sprite.centered = true;
            running_sprite.hframes = 10;
            running_sprite.vframes = 1;
            build.addNode("run",  MovingSprite::allFrames(running_sprite, 0.6f));
        }
        {
            Sprite jumping_spr = Sprite(Texture("jump-up"), def_size);
            jumping_spr.position_offset = offset;
            jumping_spr.centered = true;
            jumping_spr.hframes = 3;
            jumping_spr.vframes = 1;
            build.addNode("jump", MovingSprite::allFrames(jumping_spr, 0.25f, false));
        }
        {
            Sprite jumpfall_spr = Sprite(Texture("jumpfall"), def_size);
            jumpfall_spr.position_offset = offset;
            jumpfall_spr.centered = true;
            jumpfall_spr.hframes = 2;
            jumpfall_spr.vframes = 1;
            build.addNode("jumpfall", MovingSprite::allFrames(jumpfall_spr, 0.25f, false));
        }
        {
            Sprite falling_spr = Sprite(Texture("jump-down"), def_size);
            falling_spr.position_offset = offset;
            falling_spr.centered = true;
            falling_spr.hframes = 3;
            falling_spr.vframes = 1;
            build.addNode("fall", MovingSprite::allFrames(falling_spr, 0.25f, false));
        }

        build.addEdge("idle", "run", [](Entity owner) {
            return abs(coordinator.getComponent<Rigidbody>(owner)->velocity.x) >
                   25.f;
        });
        build.addEdge("run", "idle", [](Entity owner, bool isended) {
            return abs(coordinator.getComponent<Rigidbody>(owner)->velocity.x) <
                   25.f;
        });
        auto isJumping = [](Entity owner) {
            return coordinator.getComponent<Rigidbody>(owner)->velocity.y <
                   -100.f;
        };
        auto isFalling = [](Entity owner) {
            return coordinator.getComponent<Rigidbody>(owner)->velocity.y >
                   25.f;
        };
        auto hasFallen = [&](Entity owner) {
            return isProtagonistGrounded != false;
        };
        build.addEdge("idle", "fall", isFalling);
        build.addEdge("run", "fall", isFalling);
        build.addEdge("idle", "jump", isJumping);
        build.addEdge("run", "jump", isJumping);
        build.addEdge("jump", "jumpfall", isFalling);
        build.addEdge("jumpfall", "fall");
        build.addEdge("fall", "idle", hasFallen);
    }
    auto anim_sprite = AnimatedSprite(build);
    coordinator.addComponent(protagonist, anim_sprite);
}
void Demo::onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller) {
}
void Demo::onRender(Device&, const FrameInfo& frame) {
    ImGui::ShowDemoWindow();
}
void Demo::onUpdate(const float delta_time, Window& window, KeyboardController& controller) 
{
    EMP_LOG_INTERVAL(DEBUG2, 3.f) << "{main thread}: " << 1.f / delta_time;
    auto& phy_sys = *coordinator.getSystem<PhysicsSystem>();
    for(auto e : phy_sys.getEntities()) {
        if(!coordinator.getComponent<DebugShape>(e)) {
            continue;
        }
        if(phy_sys.m_isDormant(e)) {
            coordinator.getComponent<DebugShape>(e)->fill_color = {1, 0, 0, 1};
        }else {
            coordinator.getComponent<DebugShape>(e)->fill_color = {1, 1, 1, 1};
        }
    }
    for(auto m : markers) {
        coordinator.getComponent<Transform>(m)->position = {INFINITY, INFINITY};
        coordinator.getComponent<Transform>(m)->scale = {1, 1};
    }
    int i = 0;
    for(auto entity : phy_sys.getEntities()) {
        if(i == markers_count) {
            break;
        }
        auto& dis_set = phy_sys.m_collision_islands;
        auto other = dis_set.group(entity);

        auto& other_trans = *coordinator.getComponent<Transform>(other);
        auto& this_trans = *coordinator.getComponent<Transform>(entity);
        vec2f pos1 = this_trans.position;
        vec2f pos2 = other_trans.position;

        auto& shape = *coordinator.getComponent<Transform>(markers[i++]);
        shape.position = (pos1 + pos2) * 0.5f;
        shape.scale = vec2f(1.f, length(pos1 - pos2));
        shape.rotation = angle(vec2f(0.f, 1.f), pos1 - pos2);
    }

    {
        coordinator.getComponent<Transform>(mouse_entity)->position =
                controller.global_mouse_pos();
    }
    if(coordinator.isEntityAlive(protagonist)){
        isProtagonistGroundedSec += delta_time;
        if(isProtagonistGrounded) {
            isProtagonistGroundedSec = 0.f;
        } 
        if(coordinator.isEntityAlive(protagonist)){
            coordinator.getComponent<Rigidbody>(protagonist)->velocity.x +=
                    controller.movementInPlane2D().x *
                    (protagonist_speed / 2.f + protagonist_speed / 2.f *
                            (isProtagonistGrounded != false)) *
                    delta_time;
            auto& rb = *coordinator.getComponent<Rigidbody>(protagonist);
            if (controller.get(eKeyMappings::Jump).pressed && isProtagonistGroundedSec < cayote_time) {
                isProtagonistGroundedSec = cayote_time;
                isProtagonistGrounded = false;
                vec2f direction = vec2f(0, -10);
                direction.x = controller.movementInPlane2D().x;
                direction = normal(direction);
                rb.velocity += 500.f * direction;
            }
        }
        auto& rb = *coordinator.getComponent<Rigidbody>(protagonist);
        auto& trans = *coordinator.getComponent<Transform>(protagonist);
        bool isRunningLeft = rb.velocity.x < 0.f;
        if(abs(rb.velocity.x) > 10.f) {
            trans.scale.x = isRunningLeft ?  -1.f : 1.f;
        }
    }

    static vec2f last_pos;
    if (controller.get(eKeyMappings::Ability1).held && last_pos != vec2f(controller.global_mouse_pos())) {
        last_pos = vec2f(controller.global_mouse_pos());
        Sprite spr = Sprite(Texture("crate"), {cube_side_len, cube_side_len});
        Rigidbody rb;
        rb.useAutomaticMass = true;
        auto col = Collider(cube_model_shape);
        col.collider_layer = ITEM;
        auto entity = coordinator.createEntity();
        last_created_crate = entity;
        coordinator.addComponent(
                entity, Transform(vec2f(controller.global_mouse_pos()), 0.f)
        );
        coordinator.addComponent(entity, debug_cube_shape);
        coordinator.addComponent(entity, col);
        coordinator.addComponent(entity, rb);
        coordinator.addComponent(entity, Material());
        coordinator.addComponent(entity, spr);
    }
}
int main()
{
    {
        std::cout << sizeof(Collider);
        Demo demo;
        demo
            .run();
    }
}


