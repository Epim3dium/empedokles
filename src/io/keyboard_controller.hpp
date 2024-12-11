#ifndef EMP_KEYBOARD_CONTROLLER_HPP
#define EMP_KEYBOARD_CONTROLLER_HPP

#include <map>
#include "graphics/model_system.hpp"
#include "window.hpp"

namespace emp {
enum class eKeyMods : uint32_t {
    SHIFT = 0x0001,
    CONTROL = 0x0002,
    ALT = 0x0004,
    SUPER = 0x0008,
    CAPS_LOCK = 0x0010,
    NUM_LOCK = 0x0020,
};
struct KeyState {
    bool held = false;
    bool pressed = false;
    bool released = false;
    bool isMod(eKeyMods mod) {
        return mod_flags & (uint32_t)mod;
    }
    uint32_t mod_flags = 0;
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
class KeyboardController {
private:
    std::map<eKeyMappings, int> m_mappings;
    std::unordered_map<eKeyMappings, KeyState> m_key_states;
    static std::unordered_map<int, KeyState> keys;
    vec2f m_mouse_pos;
    vec2f m_global_mouse_pos;

public:
    vec2f mouse_pos() const {
        return m_mouse_pos;
    }
    vec2f global_mouse_pos() const {
        return m_global_mouse_pos;
    }
    void bind(eKeyMappings action, int key) {
        m_mappings[action] = key;
    }
    KeyState get(eKeyMappings action) {
        return m_key_states[action];
    }
    vec2f movementInPlane2D();
    vec2f lookingInPlane2D();
    void update(Window& window, const Transform& camera_transform);
    KeyboardController(GLFWwindow* window);
    friend void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};
} // namespace emp

#endif
