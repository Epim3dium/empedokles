#include "gui/gui_manager.hpp"
#include "imgui.h"
#include <mutex>
#include <string>
#include "gui/editor/inspector.hpp"
namespace emp  {
void GUIManager::addRendererFPS(float fps) {
    std::unique_lock<std::mutex> lock{m_access_mutex};

    auto frac = 1.f / est_count;
    FPS_renderer = frac * fps + (1.f - frac) * FPS_renderer;
}
void GUIManager::addPhysicsTPS(float tps) {
    std::unique_lock<std::mutex> lock{m_access_mutex};
    auto frac = 1.f / est_count;
    TPS_physics = frac * tps + (1.f - frac) * TPS_physics;
}
void GUIManager::addUpdateTPS(float tps) {
    std::unique_lock<std::mutex> lock{m_access_mutex};
    auto frac = 1.f / est_count;
    TPS_update = frac * tps + (1.f - frac) * TPS_update;
}
void GUIManager::alias(Entity entity, std::string name) {
    std::unique_lock<std::mutex> lock{m_access_mutex};
    auto input_name = name;
    int copy_number = 1;
    while(m_names_used.contains(name)) {
        name = input_name + '(' + std::to_string(copy_number++) + ')';
    }
    m_names_aliased[entity] = name;
    m_names_used.insert(name);
}

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
            if(ImGui::MenuItem("Entity Tree View", NULL, m_tree_view.isOpen)) m_tree_view.isOpen = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if(ImGui::MenuItem("FPS Overlay", NULL, m_FPS_overlay.isOpen)) m_FPS_overlay.isOpen = true;
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
    std::unique_lock<std::mutex> lock{m_access_mutex};
    drawMainMenuBar();
    if(m_showDemoWindow)
        ImGui::ShowDemoWindow(&m_showDemoWindow);

    m_console.draw("Console");
    m_log_window.draw("Log Window");
    m_tree_view.draw("Tree view of entities", coordinator, [&](Entity e) {
        auto& map = m_names_aliased;
        return map.find(e) == map.end() ? "entity_" + std::to_string(e) : map.at(e);
    });

    if(m_tree_view.isJustSelected()) {
        m_inspector.isOpen = true;
    }
    m_inspector.draw(m_tree_view.getVisible(), coordinator);

    m_FPS_overlay.draw([&](){
        ImGui::Text("renderer FPS: %f", FPS_renderer);
        ImGui::Text("physics  TPS: %f", TPS_physics);
        ImGui::Text("onUpdate TPS: %f", TPS_update);
    });
}
GUIManager::GUIManager() {
    m_inspector.isOpen  = false;
    m_console.isOpen    = false;
    m_log_window.isOpen = false;
    m_tree_view.isOpen  = false;
}
}
