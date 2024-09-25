#include "physics/constraint.hpp"
#include "scene/app.hpp"
#include "graphics/sprite_system.hpp"
using namespace emp;

class Demo : public App {
    public:
        Entity mouse_entity;
        Entity cube;
        // Constraint constraint;
        void onUpdate(const float delta_time, Window& window) override final {
            double xpos, ypos;
            {
                glfwGetCursorPos(window.getGLFWwindow(), &xpos, &ypos);
                auto xdenom = static_cast<float>(window.getExtent().width);
                auto ydenom = static_cast<float>( window.getExtent().height);
                xpos = (xpos + (ydenom - xdenom) / 2.f) / ydenom * 2.f - 1.f;
                ypos = ypos / ydenom * 2.f - 1.f;
                coordinator.getComponent<Transform>(mouse_entity)->position = {xpos, ypos};
            }
            // constraint.solve(delta_time);
        }
        void onRender(Device&, const FrameInfo&) override final {
        }
        std::vector<vec2f> cube_model_shape = {vec2f(-0.25f, -0.25f), vec2f(-0.25f, 0.25f), vec2f(0.25f, 0.25f), vec2f(0.25f, -0.25f)};
        void onSetup(Window& window, Device& device) override final {
            // coordinator.addComponent(cube, Model("cube"));
            cube = coordinator.createEntity();
            mouse_entity = coordinator.createEntity();
            coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));
            Sprite::create("test", Texture("dummy"), {{0, 0}, {1, 1}}).scale_offset = {0.1, 0.1};
            auto rend = SpriteRenderer("test");
            rend.flipX = true;
            coordinator.addComponent(cube, rend);

            coordinator.addComponent(cube, Transform(vec2f(0.f, 0.f), 0.f, {0.5f, 0.5f}));
            coordinator.addComponent(cube, DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)));
            coordinator.addComponent(cube, Collider(cube_model_shape)); 
            coordinator.addComponent(cube, Rigidbody()); 
            coordinator.addComponent(cube, Material()); 

            // constraint = Constraint::createPointAnchor(mouse_entity, cube);
            // constraint.stiffness = 0.5f;

            auto platform = coordinator.createEntity();
            coordinator.addComponent(platform, Transform(vec2f(0.f, 1.0f), 0.f, {4.0f, 0.5f}));
            coordinator.addComponent(platform, DebugShape(device, cube_model_shape));
            coordinator.addComponent(platform, Collider(cube_model_shape)); 
            coordinator.addComponent(platform, Rigidbody{true}); 
            coordinator.addComponent(platform, Material()); 

            // coordinator.addComponent(cube, Texture("default"));
            EMP_LOG(LogLevel::DEBUG) << "cube created, id: " << cube;
        }
        Demo() : App({{"../assets/models/colored_cube.obj", "cube"}}, {{"../assets/textures/dummy.png", "dummy"}}) {}
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

