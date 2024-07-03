set(SOURCE_FILES
    #main.cpp
    debug/log.cpp
    
    math/types.cpp
    math/math_func.cpp
    math/geometry_func.cpp

    memory/linear_allocator.cpp
    memory/pool_allocator.cpp

    scene/transform.cpp

    core/entity_manager.cpp

    physics/collider.cpp
    physics/rigidbody.cpp
    physics/physics_system.cpp
    physics/broad_phase.cpp
)
set(HEADER_FILES
    memory/parallel_iterator.hpp
    memory/pool_allocator.hpp
    memory/allocator.hpp

    debug/log.hpp
    debug/debug.hpp

    multithreading/thread_pool.hpp


    math/types.hpp
    math/shapes/AABB.hpp
    math/shapes/circle.hpp
    math/shapes/concave_polygon.hpp
    math/shapes/convex_polygon.hpp
    math/shapes/ray.hpp
    math/math_func.hpp
    math/math_defs.hpp
    math/math.hpp
    math/geometry_func.hpp

    physics/collider.hpp
    physics/rigidbody.hpp
    physics/material.hpp
    physics/physics_system.hpp
    physics/broad_phase.hpp
    
    scene/app.hpp
    scene/transform.hpp
    core/entity.hpp
    core/entity_manager.hpp
    core/component.hpp
    core/component_array.hpp
    core/component_manager.hpp
    core/system_manager.hpp
    core/coordinator.hpp

    templates/set.hpp
    templates/stack_linked_list.hpp

    debug/log.hpp
    debug/debug.hpp
)
include_directories(.)
add_library(empedokles
    ${SOURCE_FILES} ${HEADER_FILES}
)
target_compile_options(empedokles PRIVATE -glldb)

