#include "serialized_components.hpp"
#include <cstring>
namespace emp {
void SerialConvert<Transform>::encode(const Transform& var, IGlobWriter& writer) {
    writer.encode(var.m_local_transform);
    writer.encode(var.m_parents_global_transform);
    writer.encode(var.m_global_transform);
    writer.encode(var.m_parent_entity);
    writer.encode(var.m_children_entities);
    writer.encode(var.position);
    writer.encode(var.rotation);
    writer.encode(var.scale);
}
void SerialConvert<Transform>::decode(Transform& var, IGlobReader& reader) {
    reader.decode(var.m_local_transform);
    reader.decode(var.m_parents_global_transform);
    reader.decode(var.m_global_transform);
    reader.decode(var.m_parent_entity);
    reader.decode(var.m_children_entities);
    reader.decode(var.position);
    reader.decode(var.rotation);
    reader.decode(var.scale);
}

void SerialConvert<Constraint>::encode(const Constraint& var, IGlobWriter& writer) {
    writer.encode(var.entity_list);
    writer.encode(var.compliance);
    writer.encode(var.damping);
    writer.encode(var.enabled_collision_between_bodies);
    writer.encode(*(int*)&var.type);
    writer.copy(&var.data, sizeof(var.data));
}
void SerialConvert<Constraint>::decode(Constraint& var, IGlobReader& reader) {
    reader.decode(var.entity_list);
    reader.decode(var.compliance);
    reader.decode(var.damping);
    reader.decode(var.enabled_collision_between_bodies);
    reader.decode(*(int*)&var.type);
    memcpy(&var.data, reader.get(sizeof(var.data)), sizeof(var.data));
}

void SerialConvert<Material>::encode(const Material& var, IGlobWriter& writer) {
    writer.encode(var.static_friction);
    writer.encode(var.dynamic_friction);
    writer.encode(var.restitution);
    writer.encode(var.air_friction);
}
void SerialConvert<Material>::decode(Material& var, IGlobReader& reader) {
    reader.decode(var.static_friction);
    reader.decode(var.dynamic_friction);
    reader.decode(var.restitution);
    reader.decode(var.air_friction);
}

void SerialConvert<Collider>::encode(const Collider& var, IGlobWriter& writer) {
}
void SerialConvert<Collider>::decode(Collider& var, IGlobReader& reader) {
}

void SerialConvert<Rigidbody>::encode(const Rigidbody& var, IGlobWriter& writer) {
}
void SerialConvert<Rigidbody>::decode(Rigidbody& var, IGlobReader& reader) {
}

void SerialConvert<Sprite>::encode(const Sprite& var, IGlobWriter& writer) {
}
void SerialConvert<Sprite>::decode(Sprite& var, IGlobReader& reader) {
}

void SerialConvert<AnimatedSprite>::encode(const AnimatedSprite& var, IGlobWriter& writer) {
}
void SerialConvert<AnimatedSprite>::decode(AnimatedSprite& var, IGlobReader& reader) {
}
};
