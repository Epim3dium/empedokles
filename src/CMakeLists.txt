set(SOURCE_FILES
    #main.cpp
    debug/log.cpp
    
    math/types.cpp
    math/math_func.cpp
    math/geometry_func.cpp

    memory/linear_allocator.cpp
    memory/pool_allocator.cpp
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

    templates/set.hpp
    templates/stack_linked_list.hpp

    debug/log.hpp
    debug/debug.hpp
)
include_directories(.)
add_library(epimetheus
    ${SOURCE_FILES} ${HEADER_FILES}
)
target_compile_options(epimetheus PRIVATE -glldb)

