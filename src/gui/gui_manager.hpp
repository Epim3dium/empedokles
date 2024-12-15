#ifndef EMP_GUI_MANAGER_HPP
#define EMP_GUI_MANAGER_HPP

#include "core/coordinator.hpp"
#include "gui/console.hpp"
#include "gui/editor/inspector.hpp"
#include "gui/editor/tree-view.hpp"
#include "gui/log_window.hpp"
#include "gui/overlay.hpp"

namespace emp  {

class GUIManager {
    bool m_showDemoWindow = false;

    std::mutex m_access_mutex;
    std::map<Entity, std::string> m_names_aliased;
    std::set<std::string> m_names_used;

    Overlay m_FPS_overlay;
    float FPS_renderer = 0, TPS_physics = 0, TPS_update = 0;
    TreeView m_tree_view;
    Inspector m_inspector;
    Console m_console;
    LogWindow m_log_window;

    void drawMainMenuBar();
public:
    float est_count = 10.f;
    void addRendererFPS(float fps);
    void addPhysicsTPS(float tps);
    void addUpdateTPS(float tps);

    void alias(Entity entity, std::string name);
    void draw(Coordinator& coordinator);
    GUIManager();
};

}

#endif // !DEBUG
