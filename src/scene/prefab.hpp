#ifndef EMP_PREFAB_HPP
#define EMP_PREFAB_HPP
#include "core/coordinator.hpp"
#include "io/serializer.hpp"
namespace emp {
class Prefab {
    Coordinator environment;
    void write(IBlobWriter& writer);
    void read(IBlobReader& reader);
    Prefab(Coordinator& ECS, Entity prefab_source);
};
};
#endif
