#ifndef EMP_GUI_MANAGER_HPP
#define EMP_GUI_MANAGER_HPP

#include "core/coordinator.hpp"
#include "gui/console.hpp"
#include "gui/editor/inspector.hpp"
#include "gui/log_window.hpp"

namespace emp  {

class GUIManager {
    Console m_console;
    LogWindow m_log_window;
    void drawConsole();
public:
    void draw(Coordinator& coordinator);
};

}

#endif // !DEBUG
