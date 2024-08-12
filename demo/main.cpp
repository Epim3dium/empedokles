#include "scene/app.hpp"
using namespace emp;

void onRender(Device&, const FrameInfo&) {
    EMP_LOG_DEBUG << "hello world";
}
void onSetup(Window& window, Device& device) {
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
int main()
{
    {
        App demo;
        demo
            .addBehaviour(onSetup)
            .addAssetModel("../assets/models/colored_cube.obj", "cube")
            .run();
    }
    return 0;
}

