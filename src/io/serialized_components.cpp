#include "serialized_components.hpp"
namespace emp {
void SerialConvert<Transform>::encode(const Transform& var, IGlobWriter& writer) {
    writer.encode(var.local());
    writer.encode(var.m_parents_global_transform);
    writer.encode(var.global());
    writer.encode(var.parent());
    writer.encode(var.children());
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
}
void SerialConvert<Constraint>::decode(Constraint& var, IGlobReader& reader) {
}

void SerialConvert<Material>::encode(const Material& var, IGlobWriter& writer) {
}
void SerialConvert<Material>::decode(Material& var, IGlobReader& reader) {
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
