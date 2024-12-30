#include "gui/gui_manager.hpp"
#include "imgui.h"
#include <mutex>
#include <string>
#include "gui/editor/inspector.hpp"
#include "scene/scene_defs.hpp"
namespace emp  {
void GUIManager::addRendererTime(float time) {
    std::unique_lock<std::mutex> lock{m_access_mutex};

    auto frac = 1.f / estimation_count;
    renderer_time = frac * time + (1.f - frac) * renderer_time;
}
void GUIManager::addPhysicsTime(float time) {
    std::unique_lock<std::mutex> lock{m_access_mutex};
    auto frac = 1.f / estimation_count;
    physics_time = frac * time + (1.f - frac) * physics_time;
}
void GUIManager::addUpdateTime(float time) {
    std::unique_lock<std::mutex> lock{m_access_mutex};
    auto frac = 1.f / estimation_count;
    mainUpdate_time = frac * time + (1.f - frac) * mainUpdate_time;
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
            if(ImGui::MenuItem("Demo", NULL, m_showDemoWindow)) m_showDemoWindow = !m_showDemoWindow;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if(ImGui::MenuItem("Entity Tree View", NULL, m_tree_view.isOpen)) m_tree_view.isOpen = !m_tree_view.isOpen;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if(ImGui::MenuItem("FPS Overlay", NULL, m_FPS_overlay.isOpen)) m_FPS_overlay.isOpen = !m_FPS_overlay.isOpen;
            if(ImGui::MenuItem("Show entities", NULL, m_visualizer.isOpen)) m_visualizer.isOpen = !m_visualizer.isOpen;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            if(ImGui::MenuItem("Log Window", NULL, m_log_window.isOpen)) m_log_window.isOpen = !m_log_window.isOpen;
            if(ImGui::MenuItem("Console", NULL, m_console.isOpen)) m_console.isOpen = !m_console.isOpen;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void GUIManager::draw(Coordinator& coordinator, Camera& camera) {
    std::unique_lock<std::mutex> lock{m_access_mutex};
    drawMainMenuBar();
    if(m_showDemoWindow)
        ImGui::ShowDemoWindow(&m_showDemoWindow);

    m_console.draw("Console");
    m_log_window.draw("Log Window");
    auto naming_function = [&](Entity e) {
        auto& map = m_names_aliased;
        return map.find(e) == map.end() ? "entity_" + std::to_string(e) : map.at(e);
    };
    m_tree_view.draw("Tree view of entities", coordinator, naming_function);

    if(m_tree_view.isJustSelected()) {
        m_inspector.isOpen = true;
        m_entity_selected = m_tree_view.getSelected();
        
        inspectorMaster = &m_tree_view.isOpen;
        m_inspector.isOpen = true;
    }
    if(inspectorMaster) {
        if(!m_inspector.isOpen || !*inspectorMaster) {
            *inspectorMaster = false;
            m_inspector.isOpen = false;
            inspectorMaster = nullptr;
        }
    }
    m_inspector.draw(m_tree_view.getSelected(), coordinator);

    m_FPS_overlay.draw([&](){
        auto total_time = mainUpdate_time;
        total_time += EMP_ENABLE_PHYSICS_THREAD * renderer_time;
        total_time += EMP_ENABLE_PHYSICS_THREAD * physics_time;

        if(EMP_ENABLE_RENDER_THREAD) {
            ImGui::Text("renderer FPS: %.4g", 1.0 / renderer_time);
        }else {
            ImGui::Text("render time: %.3g%%", floorf(renderer_time / total_time * 100.f));
        }
        if(EMP_ENABLE_PHYSICS_THREAD) {
            ImGui::Text("physics  TPS: %.4g", 1.0 / physics_time);
        }else {
            ImGui::Text("physics  time: %.3g%%", floorf(physics_time / total_time * 100.f));
        }
        ImGui::Text("mainLoop TPS: %.4g", 1.0 / mainUpdate_time);
    });
    m_visualizer.draw("visualizer", coordinator, naming_function, camera);
}
GUIManager::GUIManager() {
    m_inspector.isOpen  = false;
    m_console.isOpen    = false;
    m_log_window.isOpen = false;
    m_tree_view.isOpen  = false;
    m_visualizer.isOpen = false;
}
}
