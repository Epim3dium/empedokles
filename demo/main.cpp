#include "scene/app.hpp"
using namespace emp;

class Demo : public App {
    public:
        void onRender(Device&, const FrameInfo&) override final {
        }
        void onSetup(Window& window, Device& device) override final {
            auto cube = coordinator.createEntity();
            coordinator.addComponent(cube, Transform(vec2f(0.f, 0.5f), 0.f, {0.5f, 0.5f}));
            // coordinator.addComponent(cube, Model("cube"));
            auto triangle = coordinator.createEntity();
            coordinator.addComponent(triangle, Transform(vec2f(0.f, 0.0f), 0.f, {1.0f, 1.0f}));
            auto shape = DebugShape(device, {vec2f(-0.25f, 0.f), vec2f(0.f, 0.25f), vec2f(0.25f, 0.f), vec2f(0.f, -0.25f)});
            shape.fill_color = glm::vec4(1, 0, 0, 1);
            coordinator.addComponent(triangle, shape);
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

