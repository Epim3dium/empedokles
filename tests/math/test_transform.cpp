
#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"
#include "math/shapes/circle.hpp"
#include "scene/transform.hpp"

using namespace emp;
TEST(TransformTest, Updates) {
    Transform trans(vec2f(0, 0), 0.f, vec2f(1, 1));
    vec2f offset = vec2f(10, 10);
    trans.position += offset;
    trans.syncWithChange();
    vec2f point = {0, 5};
    vec2f tr_point = transformPoint(trans.global(), point);
    ASSERT_EQ(tr_point, point + offset);
}
