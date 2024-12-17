# 🎯 TODO:

- collision
    - more constraints+
    - collision optimization
        - tighter locked critical section
- graphics
    - instanced rendering
    - 2d lighting
- core
    - serialization / deserialization
    - message bus
- networking?

---

# 📌 DOING:

---

# ⚠️ ISSUES:
- weird error message when closing window
- when spawing overlapping colliders they explode
- 'drifting' physics problem
---

# ✅ DONE:

- collision
    - added more constraints
- graphics
    - imgui inspector for basic components
    - basic animation system
        - truly moving animated sprites
    - add imgui
- collision
    - collision optimization
        - collision idle state
    - collision triggers
- graphics
    - gui - mainly for debuggin for now
    - resizable window
    - render system abstraction
    - basic animation system
        - animated sprites
        - finite state machine animated sprite switching
- separate threads for rendering, physics and the rest
- constraints as components
- sprites (color, texRect, origin, render pipeline)

- ##### basics of rendering system
- ##### basics of collision system
- ##### basics of ECS system
