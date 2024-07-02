set(SOURCE_FILES
    #main.cpp
    debug/log.cpp
    
    math/types.cpp
    math/math_func.cpp
    math/geometry_func.cpp

    memory/linear_allocator.cpp
    memory/pool_allocator.cpp

    core/transform.cpp
    core/app.cpp

    physics/collider.cpp
    physics/rigidbody.cpp
    physics/physics_system.cpp
    physics/broad_phase.cpp

    graphics/systems/point_light_system.cpp
    graphics/systems/simple_render_system.cpp

    graphics/vulkan/buffer.cpp
    graphics/vulkan/descriptors.cpp
    graphics/vulkan/device.cpp
    graphics/vulkan/pipeline.cpp
    graphics/vulkan/swap_chain.cpp

    graphics/camera.cpp
    graphics/game_object.cpp
    graphics/model.cpp
    graphics/renderer.cpp
    graphics/texture.cpp

    io/keyboard_movement_controller.cpp
    io/window.cpp
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
    
    core/app.hpp
    core/transform.hpp

    templates/set.hpp
    templates/stack_linked_list.hpp

    debug/log.hpp
    debug/debug.hpp

    graphics/systems/point_light_system.hpp
    graphics/systems/simple_render_system.hpp

    graphics/vulkan/buffer.hpp
    graphics/vulkan/descriptors.hpp
    graphics/vulkan/device.hpp
    graphics/vulkan/pipeline.hpp
    graphics/vulkan/swap_chain.hpp

    graphics/camera.hpp
    graphics/frame_info.hpp
    graphics/game_object.hpp
    graphics/model.hpp
    graphics/renderer.hpp
    graphics/texture.hpp
    graphics/utils.hpp

    io/keyboard_movement_controller.hpp
    io/window.hpp

    external/tinyobjloader/tiny_obj_loader.h
    external/stbimage/stb_image.h
)
include_directories(.)
include_directories(external/tinyobjloader)
include_directories(external/stbimage)
add_library(empedokles
    ${SOURCE_FILES} ${HEADER_FILES}
)
target_compile_options(empedokles PRIVATE -glldb)

