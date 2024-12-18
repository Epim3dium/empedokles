# 🎯 TODO:

- collision
    - more constraints
    - collision optimization
        - tighter locked critical section
- graphics
    - separate command queue for ui only changed when imgui is interacted with
    - expand imgui editor
    - 2d lighting
- core
    - message bus
    - command terminal

---

# 📌 DOING:
- core
    - reconsider global coordinator

---

# ⚠️ ISSUES:
- when spawing overlapping colliders they explode
- restitution unreliability
---

# ✅ DONE:

- weird error message when closing window
- graphics
    - basic animation system
        - truly moving animated sprites
    - add imgui
- collision
    - collision optimization
        - collision idle state
- graphics
    - basic animation system
        - truly moving animated sprites
    - add imgui
- collision
    - collision optimization
        - collision idle state
- graphics
    - resizable window
    - render system abstraction
    - basic animation system
        - animated sprites
        - finite state machine animated sprite switching
- collision
    - collision triggers
- separate threads for rendering, physics and the rest
- constraints as components
- sprites (color, texRect, origin, render pipeline)

- ##### basics of rendering system
- ##### basics of collision system
- ##### basics of ECS system
