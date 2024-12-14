#ifndef EMP_GUI_MANAGER_HPP
#define EMP_GUI_MANAGER_HPP

#include "core/coordinator.hpp"
#include "gui/console.hpp"
#include "gui/editor/inspector.hpp"
#include "gui/editor/tree-view.hpp"
#include "gui/log_window.hpp"

namespace emp  {

class GUIManager {
    bool m_showDemoWindow = false;

    std::map<Entity, std::string> m_names_aliased;
    std::set<std::string> m_names_used;

    TreeView m_tree_view;
    Inspector m_inspector;
    Console m_console;
    LogWindow m_log_window;
    void drawConsole();
    void drawMainMenuBar();
public:
    void alias(Entity entity, std::string name);
    void draw(Coordinator& coordinator);
    GUIManager();
};

}

#endif // !DEBUG
