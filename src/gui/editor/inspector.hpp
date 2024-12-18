#ifndef EMP_INSPECTOR_HPP
#define EMP_INSPECTOR_HPP
#include "core/entity.hpp"
#include "scene/register_scene_types.hpp"
#include "templates/type_pack.hpp"
namespace emp {
template<class Vec>
float* VecToPtr(Vec& vec) {
    return &vec.x;
}
class Inspector {
    void DisplayMat4(const glm::mat4& mat, const char* label = "unnamed") {
        ImGui::Text("%s", label);
        if (ImGui::BeginTable(label, 4, ImGuiTableFlags_Borders)) {
            for (int row = 0; row < 4; ++row) {
                ImGui::TableNextRow();
                for (int col = 0; col < 4; ++col) {
                    ImGui::TableSetColumnIndex(col);
                    ImGui::Text("% .3f", mat[row][col]);
                }
            }
            ImGui::EndTable();
        }
    }
    template<class T>
    std::string prettyName() {
        std::string result(typeid(T).name());
        int found = -1;
        for (char ch = '0'; ch <= '9'; ch++) {
            found = std::max(found, (int)result.rfind(ch));
        }
        if(found == -1)
            return result;
        return result.substr(found + 1);
    }

    template<class CompType>
    void inspectProxy(Entity e) {
        auto* comp = ECS.getComponent<CompType>(e);
        if(comp == nullptr) {
            return;
        }
        auto header_name = prettyName<CompType>();
        if (ImGui::CollapsingHeader(header_name.c_str())) {
            ImGui::Indent();
            inspect(e, *comp);
            ImGui::Unindent();
        }
    }
    template<class CompType>
    void inspect(Entity e, CompType& type) {
        EMP_LOG_INTERVAL(WARNING, 1.f) << "Component: '" << typeid(CompType).name() << "' has no inspect implemented";
    }
    template<> 
    void inspect<Transform>(Entity e, Transform& transform) {
        ImGui::DragFloat2("position", VecToPtr(transform.position));
        ImGui::DragFloat("rotation", &transform.rotation, 0.01f, -M_PI * 2.f, M_PI * 2.f);
        ImGui::DragFloat2("scale", VecToPtr(transform.scale), 0.1f);
        DisplayMat4(transform.local(), "local transform");
        DisplayMat4(transform.global(), "global transform");
    }
    template<> 
    void inspect<Material>(Entity e, Material& material) {
        ImGui::DragFloat("restitution", &material.restitution, 0.1f);
        ImGui::DragFloat("static_fric ", &material.static_friction, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("dynamic_fric", &material.dynamic_friction, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("air_friction", &material.air_friction, 0.1f, 0.f, INFINITY);
    }
    template<> 
    void inspect<Collider>(Entity e, Collider& collider) {
        ImGui::Checkbox("isNonMoving", &collider.isNonMoving);
        const uint8_t min_layers = 0;
        const uint8_t max_layers = MAX_LAYERS - 1;
        ImGui::SliderScalar("collider layer", ImGuiDataType_U8, &collider.collider_layer, &min_layers, &max_layers);
    }
    template<> 
    void inspect<Rigidbody>(Entity e, Rigidbody& rigidbody) {
        ImGui::Checkbox("isStatic", &rigidbody.isStatic);
        ImGui::Checkbox("isSleeping", &rigidbody.isSleeping);
        ImGui::Checkbox("lock rotation", &rigidbody.isRotationLocked);
        ImGui::Checkbox("use auto mass", &rigidbody.useAutomaticMass);
        ImGui::DragFloat2("velocity", VecToPtr(rigidbody.velocity));
        ImGui::DragFloat("angular_vel", &rigidbody.angular_velocity, M_PI / 50.f);
    }
    template<> 
    void inspect<DebugShape>(Entity e, DebugShape& shape) {
        ImGui::ColorEdit4("shape fill color", VecToPtr(shape.fill_color));
        ImGui::ColorEdit4("shape outline color", VecToPtr(shape.outline_color));
    }
    template<> 
    void inspect<AnimatedSprite>(Entity e, AnimatedSprite& sprite) {
        ImGui::Checkbox("flip horizontal", &sprite.flipX);
        ImGui::Checkbox("flip vertical", &sprite.flipY);
        ImGui::ColorEdit4("sprite color", VecToPtr(sprite.color));
        ImGui::SliderFloat("speed", &sprite.animation_speed, 0.f, 10.f);
        ImGui::DragFloat2("offset", VecToPtr(sprite.position_offset));
    }

    template<class ...Ts>
    void inspectAll(Entity e, TypePack<Ts...>) {
        (inspectProxy<Ts>(e), ...);
    }
public:
    Inspector(Entity e) {
        inspectAll(e, AllComponentTypes());
    }
};
};
#endif //EMP_INSPECTOR_HPP
