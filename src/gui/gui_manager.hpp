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
    TreeView m_tree_view;
    Console m_console;
    LogWindow m_log_window;
    void drawConsole();
    void drawMainMenuBar();
public:
    void draw(Coordinator& coordinator);
    GUIManager();
};

}

#endif // !DEBUG
