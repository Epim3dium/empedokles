#ifndef EMP_KEYBOARD_CONTROLLER_HPP
#define EMP_KEYBOARD_CONTROLLER_HPP

#include "graphics/model_system.hpp"
#include "window.hpp"
#include <map>

namespace emp {
    struct KeyState {
        bool held = false;
        bool pressed = false;
        bool released = false;
    };
    enum class eKeyMappings {
        LookLeft,
        LookRight,
        LookUp,
        LookDown,

        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,

        Jump,
        Dash,
        Sprint,
        Crouch,

        Shoot,
        Ability1,
        Ability2,
        Ability3,
        Ability4,
        Ability5,
        Ability6,
        Ability7,
        Ability8,
    };
    class KeyboardControllerSystem;
    class KeyboardController {
    private:
        std::map<eKeyMappings, int> m_mappings;
        std::unordered_map<eKeyMappings, KeyState> m_key_states;
    public:
        void bind(eKeyMappings action, int key) {
            m_mappings[action] = key;
        }
        KeyState get(eKeyMappings action) {
            return m_key_states[action];
        }
        vec2f movementInPlane2D();
        friend KeyboardControllerSystem;
    };

    class KeyboardControllerSystem : public System<KeyboardController> {
    private:
        std::unordered_map<int, KeyState> keys;
    public:
        inline const std::unordered_map<int, KeyState>& getKeys() const {
            return keys;
        }
        void update(GLFWwindow* window);
    };
}  // namespace emp

#endif
