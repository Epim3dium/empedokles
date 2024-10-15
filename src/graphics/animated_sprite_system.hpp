#ifndef EMP_ANIMATED_SPRITE_SYSTEM
#define EMP_ANIMATED_SPRITE_SYSTEM
#include "graphics/animated_sprite.hpp"
#include "scene/transform.hpp"
namespace emp {
    class AnimatedSriteSystem : public System<AnimatedSprite, Transform> {
    };
};
#endif //EMP_ANIMATED_SPRITE_SYSTEM
