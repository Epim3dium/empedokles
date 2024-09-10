#include "scene/app.hpp"
using namespace emp;

class Demo : public App {
    public:
        void onRender(Device&, const FrameInfo&) override final {
        }
        std::vector<vec2f> cube_model_shape = {vec2f(-0.25f, -0.25f), vec2f(-0.25f, 0.25f), vec2f(0.25f, 0.25f), vec2f(0.25f, -0.25f)};
        void onSetup(Window& window, Device& device) override final {
            // coordinator.addComponent(cube, Model("cube"));
            auto cube = coordinator.createEntity();
            coordinator.addComponent(cube, Transform(vec2f(0.f, 0.0f), 0.f, {0.5f, 0.5f}));
            coordinator.addComponent(cube, DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)));
            coordinator.addComponent(cube, Collider(cube_model_shape)); 
            coordinator.addComponent(cube, Rigidbody()); 
            coordinator.addComponent(cube, Material()); 
            auto platform = coordinator.createEntity();
            coordinator.addComponent(platform, Transform(vec2f(0.f, 1.0f), 0.f, {4.0f, 0.5f}));
            coordinator.addComponent(platform, DebugShape(device, cube_model_shape));
            coordinator.addComponent(platform, Collider(cube_model_shape)); 
            coordinator.addComponent(platform, Rigidbody{true}); 
            coordinator.addComponent(platform, Material()); 

            // coordinator.addComponent(cube, Texture("default"));
            EMP_LOG(LogLevel::DEBUG) << "cube created, id: " << cube;
        }
        Demo() : App({{"../assets/models/colored_cube.obj", "cube"}}, {}) {}
};
int main()
{
    {
        Demo demo;
        demo
            .run();
    }
    return 0;
}

