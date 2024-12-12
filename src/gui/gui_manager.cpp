#include "gui/gui_manager.hpp"
namespace emp  {

void GUIManager::draw(Coordinator& coordinator) {
    m_console.draw("Console");
    m_log_window.draw("Log Window");
}

}
