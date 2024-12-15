#ifndef EMP_TREE_VIEW_HPP
#define EMP_TREE_VIEW_HPP

#include "imgui.h"
#include <stack>
#include <string>
#include <vector>
#include "core/coordinator.hpp"
#include "core/entity.hpp"
#include "debug/log.hpp"
#include "scene/transform.hpp"
namespace emp  {
class TreeView {
private:
    struct TreeNode {
        Entity entity;
        std::vector<TreeNode> children;
    };
    TreeNode m_root;
    Entity visible_entity = 0;
    bool just_selected = false;

    void log(TreeNode& node, int indent = 0) {
        for (auto& child : node.children ) {
            std::string tab = "";
            for(int i = 0; i < indent; i++) tab += '\t';
            EMP_LOG_DEBUG << tab << child.entity;
            log(child, indent + 1);
        }
    }
    void constructTree(Coordinator& ECS) {
        m_root.children.clear();
        std::stack<std::pair<Entity, TreeNode*>> to_process;
        to_process.push({ECS.world(), &m_root});

        while(!to_process.empty()) {
            auto [entity, node] = to_process.top();
            to_process.pop();
            auto* trans = ECS.getComponent<Transform>(entity);
            if(trans == nullptr)
                continue;
            const auto& children = trans->children();
            for(auto child : children) {
                node->children.push_back({child, {}});
            }
            int i = 0;
            for(auto child : children) {
                to_process.push({child, &node->children[i++]});
            }
        }
    }
    void drawTreeNode(TreeNode& node, std::function<std::string(Entity)> dispFunc ) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushID(node.entity);
        ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
        tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;    // Standard opening mode as we are likely to want to add selection afterwards
        tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;                                  // Left arrow support
        if (node.entity == visible_entity)
            tree_flags |= ImGuiTreeNodeFlags_Selected;
        if (node.children.size() == 0)
            tree_flags |= ImGuiTreeNodeFlags_Leaf;
        //DISPLAY COMPONENTS
        std::string name = "entity_" + std::to_string(node.entity);
        if(dispFunc != nullptr)
            name = dispFunc(node.entity);
        bool node_open = ImGui::TreeNodeEx("", tree_flags, "%s", name.c_str());
        auto prev_visible = visible_entity;
        if (ImGui::IsItemFocused()) {
            visible_entity = node.entity;
        }
        if(prev_visible != visible_entity) {
            just_selected = true;
        }
        if (node_open)
        {
            for (TreeNode& child : node.children)
                drawTreeNode(child, dispFunc);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
public:
    Entity getSelected() const { return visible_entity; }
    bool isJustSelected() const { return just_selected; }
    bool isOpen = true;
    void log() {
        log(m_root, 1);
    }
    void draw(const char* title, Coordinator& ECS, std::function<std::string(Entity)> dispFunc = nullptr) {
        if(!isOpen)return;

        just_selected = false;
        constructTree(ECS);

        if (ImGui::Begin(title, &isOpen))
        {
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);

            if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
            {
                for (auto& node : m_root.children)
                    drawTreeNode(node, dispFunc);
                ImGui::EndTable();
            }
            ImGui::End();
        }

    }
    TreeView() {
    }
};

}
#endif  //EMP_TREE_VIEW_HPP
