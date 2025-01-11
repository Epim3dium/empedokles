#include "serialized_components.hpp"
#include "io/serial_convert.hpp"
#include "serializer.hpp"
#include <cstring>
namespace emp {
void SerialConvert<Transform>::encode(const Transform& var, IBlobWriter& writer) {
    writer.encode(var.m_local_transform);
    writer.encode(var.m_parents_global_transform);
    writer.encode(var.m_global_transform);
    writer.encode(var.m_parent_entity);
    writer.encode(var.m_children_entities);
    writer.encode(var.position);
    writer.encode(var.rotation);
    writer.encode(var.scale);
}
void SerialConvert<Transform>::decode(Transform& var, IBlobReader& reader) {
    reader.decode(var.m_local_transform);
    reader.decode(var.m_parents_global_transform);
    reader.decode(var.m_global_transform);
    reader.decode(var.m_parent_entity);
    reader.decode(var.m_children_entities);
    reader.decode(var.position);
    reader.decode(var.rotation);
    reader.decode(var.scale);
}

void SerialConvert<Constraint>::encode(const Constraint& var, IBlobWriter& writer) {
    writer.encode(var.entity_list);
    writer.encode(var.compliance);
    writer.encode(var.damping);
    writer.encode(var.enabled_collision_between_bodies);
    writer.encode(*(int*)&var.type);
    writer.copy(&var.data, sizeof(var.data));
}
void SerialConvert<Constraint>::decode(Constraint& var, IBlobReader& reader) {
    reader.decode(var.entity_list);
    reader.decode(var.compliance);
    reader.decode(var.damping);
    reader.decode(var.enabled_collision_between_bodies);
    reader.decode(*(int*)&var.type);
    memcpy(&var.data, reader.get(sizeof(var.data)), sizeof(var.data));
}

void SerialConvert<Material>::encode(const Material& var, IBlobWriter& writer) {
    writer.encode(var.static_friction);
    writer.encode(var.dynamic_friction);
    writer.encode(var.restitution);
    writer.encode(var.air_friction);
}
void SerialConvert<Material>::decode(Material& var, IBlobReader& reader) {
    reader.decode(var.static_friction);
    reader.decode(var.dynamic_friction);
    reader.decode(var.restitution);
    reader.decode(var.air_friction);
}

void SerialConvert<Collider>::encode(const Collider& var, IBlobWriter& writer) {
    writer.encode(var.m_aabb);
    writer.encode(var.m_model_outline);
    writer.encode(var.m_model_shape);
    writer.encode(var.m_transformed_outline);
    writer.encode(var.m_transformed_shape);
    writer.encode(var.collider_layer);
    writer.encode(var.isNonMoving);
}
void SerialConvert<Collider>::decode(Collider& var, IBlobReader& reader) {
    reader.decode(var.m_aabb);
    reader.decode(var.m_model_outline);
    reader.decode(var.m_model_shape);
    reader.decode(var.m_transformed_outline);
    reader.decode(var.m_transformed_shape);
    reader.decode(var.collider_layer);
    reader.decode(var.isNonMoving);
}

void SerialConvert<Rigidbody>::encode(const Rigidbody& var, IBlobWriter& writer) {
    writer.encode(var.prev_pos);
    writer.encode(var.velocity_pre_solve);
    writer.encode(var.prev_rot);
    writer.encode(var.ang_velocity_pre_solve);
    writer.encode(var.real_inertia);
    writer.encode(var.real_mass);
    writer.encode(var.real_density);
    writer.encode(var.isStatic);
    writer.encode(var.isRotationLocked);
    writer.encode(var.isSleeping);
    writer.encode(var.useAutomaticMass);
    writer.encode(var.velocity);
    writer.encode(var.force);
    writer.encode(var.angular_velocity);
    writer.encode(var.torque);
}
void SerialConvert<Rigidbody>::decode(Rigidbody& var, IBlobReader& reader) {
    reader.decode(var.prev_pos);
    reader.decode(var.velocity_pre_solve);
    reader.decode(var.prev_rot);
    reader.decode(var.ang_velocity_pre_solve);
    reader.decode(var.real_inertia);
    reader.decode(var.real_mass);
    reader.decode(var.real_density);
    reader.decode(var.isStatic);
    reader.decode(var.isRotationLocked);
    reader.decode(var.isSleeping);
    reader.decode(var.useAutomaticMass);
    reader.decode(var.velocity);
    reader.decode(var.force);
    reader.decode(var.angular_velocity);
    reader.decode(var.torque);
}

void SerialConvert<Texture>::encode(const Texture& var, IBlobWriter& writer) {
    std::string id = var.getID();
    writer.encode(id);
}
void SerialConvert<Texture>::decode(Texture& var, IBlobReader& reader) {
    std::string id;
    reader.decode(id);
    var = Texture(id);
}

void SerialConvert<Sprite>::encode(const Sprite& var, IBlobWriter& writer) {
    writer.encode(var.m_texture);
    writer.encode(var.m_rect);
    writer.encode(var.m_size);
    writer.encode(var.vframes);
    writer.encode(var.hframes);
    writer.encode(var.frame);
    writer.encode(var.centered);
    writer.encode(var.flipX);
    writer.encode(var.flipY);
    writer.encode(var.color);
    writer.encode(var.isOverridingColor);
    writer.encode(var.color_override);
}
void SerialConvert<Sprite>::decode(Sprite& var, IBlobReader& reader) {
    reader.decode(var.m_texture);
    reader.decode(var.m_rect);
    reader.decode(var.m_size);
    reader.decode(var.vframes);
    reader.decode(var.hframes);
    reader.decode(var.frame);
    reader.decode(var.centered);
    reader.decode(var.flipX);
    reader.decode(var.flipY);
    reader.decode(var.color);
    reader.decode(var.isOverridingColor);
    reader.decode(var.color_override);
}
template<>
struct SerialConvert<MovingSprite::FrameDuration> {
    void encode(const MovingSprite::FrameDuration& var, IBlobWriter& writer) {
        writer.encode(var.frame);
        writer.encode(var.duration);
    }
    void decode(MovingSprite::FrameDuration& var, IBlobReader& reader) {
        reader.decode(var.frame);
        reader.decode(var.duration);
    }
};
void SerialConvert<MovingSprite>::encode(const MovingSprite& var, IBlobWriter& writer) {
    writer.encode(var.sprite);
    writer.encode(var.isLooping);
    writer.encode(var.frames);
}
void SerialConvert<MovingSprite>::decode(MovingSprite& var, IBlobReader& reader) {
    reader.decode(var.sprite);
    reader.decode(var.isLooping);
    reader.decode(var.frames);
}

void SerialConvert<AnimatedSprite>::encode(const AnimatedSprite& var, IBlobWriter& writer) {
    writer.encode(var.m_machine_handle);
    writer.encode(var.m_moving_sprites);
    writer.encode(var.m_anim_state);
    writer.encode(var.animation_speed);
    writer.encode(var.position_offset);
    writer.encode(var.flipX);
    writer.encode(var.flipY);
    writer.encode(var.color);
    writer.encode(var.isOverridingColor);
    writer.encode(var.color_override);
}
void SerialConvert<AnimatedSprite>::decode(AnimatedSprite& var, IBlobReader& reader) {
    reader.decode(var.m_machine_handle);
    reader.decode(var.m_moving_sprites);
    reader.decode(var.m_anim_state);
    reader.decode(var.animation_speed);
    reader.decode(var.position_offset);
    reader.decode(var.flipX);
    reader.decode(var.flipY);
    reader.decode(var.color);
    reader.decode(var.isOverridingColor);
    reader.decode(var.color_override);
}
void SerialConvert<Model>::encode(const Model& var, IBlobWriter& writer) {
}
void SerialConvert<Model>::decode(Model& var, IBlobReader& reader) {
}
};
