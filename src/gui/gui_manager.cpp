#include "gui/gui_manager.hpp"
#include "imgui.h"
namespace emp  {

void GUIManager::drawMainMenuBar() {
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Demo", NULL, m_showDemoWindow)) m_showDemoWindow = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            if(ImGui::MenuItem("Log Window", NULL, m_log_window.isOpen)) m_log_window.isOpen = true;
            if(ImGui::MenuItem("Console", NULL, m_console.isOpen)) m_console.isOpen = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void GUIManager::draw(Coordinator& coordinator) {
    drawMainMenuBar();
    if(m_showDemoWindow)
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    m_console.draw("Console");
    m_log_window.draw("Log Window");
    m_tree_view.draw("Tree view of entities", coordinator);
}
GUIManager::GUIManager() {
    m_console.isOpen = false;
    m_log_window.isOpen = false;
}

}
