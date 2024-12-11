#ifndef EMP_GUI_MANAGER_HPP
#define EMP_GUI_MANAGER_HPP

#include "core/coordinator.hpp"
#include "gui/console.hpp"

namespace emp  {

class GUIManager {
    Console m_console;
    void drawConsole();
public:
    void draw(Coordinator& coordinator);
};

}

#endif // !DEBUG
