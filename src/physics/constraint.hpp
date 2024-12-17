#ifndef EMP_CONSTRAINT_HPP
#define EMP_CONSTRAINT_HPP
#include <cmath>
#include <functional>
#include <set>
#include <vector>
#include "core/coordinator.hpp"
#include "core/entity.hpp"
#include "core/system.hpp"
#include "math/math_defs.hpp"
#include "physics/rigidbody.hpp"
#include "scene/transform.hpp"
namespace emp {
enum class eConstraintType {
    Undefined,
    SwivelPoint, // 2bodies
    SwivelPointAnchored, // 2bodies
};
struct PositionalCorrectionInfo {
    Entity entity1;
    vec2f center_to_collision1;
    bool isStatic1;
    float inertia1;
    float mass1;
    float generalized_inverse_mass1;
    Entity entity2;
    vec2f center_to_collision2;
    bool isStatic2;
    float inertia2;
    float mass2;
    float generalized_inverse_mass2;
    PositionalCorrectionInfo() {
    }
    PositionalCorrectionInfo(
            vec2f normal,
            Entity e1,
            vec2f r1,
            const Rigidbody* rb1,
            Entity e2,
            vec2f r2,
            const Rigidbody* rb2 = nullptr
    );
};
struct PositionalCorrResult {
    vec2f pos1_correction = vec2f(0, 0);
    float rot1_correction = 0.f;
    vec2f pos2_correction = vec2f(0, 0);
    float rot2_correction = 0.f;

    float delta_lagrange = 0.f;
};
PositionalCorrResult calcPositionalCorrection(
        PositionalCorrectionInfo info,
        float c,
        vec2f normal,
        float delT,
        float compliance = 0.f
);
struct Constraint {
    std::vector<Entity> entity_list;
    float compliance = 0.0;
    float damping = 1.f;
    bool enabled_collision_between_bodies = false;
    eConstraintType type = eConstraintType::Undefined;

    union {
        struct {
            vec2f anchor_point_model;
            vec2f pinch_point_model;
        } swivel_anchored;
        struct {
            vec2f pinch_point_model1;
            vec2f pinch_point_model2;
        } swivel_dynamic;
    };
    struct Builder {
    private:
        std::pair<Entity, const Transform*> anchor = {-1, nullptr};
        std::vector<std::pair<Entity, const Transform*>> entity_list;
        float compliance = 0.0;
        float damping = 1.f;
        eConstraintType type = eConstraintType::Undefined;
        vec2f global_point = {NAN, NAN};
        vec2f point_rel1 = {NAN, NAN};
        vec2f point_rel2 = {NAN, NAN};

        bool enabled_collision_between_bodies = false;
    public:
        Builder& addConstrainedEntity(Entity entity, const Transform&);
        Builder& addAnchorEntity(Entity entity, const Transform&);

        Builder& setCompliance(float compliance);
        Builder& setDamping(float damping);

        Builder& enableCollision(bool enable = true);

        Builder& setConnectionGlobalPoint(vec2f point);
        Builder& setConnectionRelativePoint(vec2f point_rel1, vec2f point_rel2);

        Constraint build();
    };

    void solve(float delta_time, Coordinator& ECS);

private:
    void m_solvePointAnchor(float delta_time, Coordinator& ECS);
    void m_solvePointSwivel(float delta_time, Coordinator& ECS);
};
struct ConstraintSystem : public System<Constraint> {
    typedef const std::vector<Entity>* EntityListRef_t;
    std::vector< EntityListRef_t> getConstrainedGroups() const;
    void update(float delta_time);
};
}; // namespace emp
#endif

