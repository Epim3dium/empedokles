#include "templates/relative_vector.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace emp;

TEST(RelativeVectorTest, Initialization) {
    RelativeVector<int> vec = {1, 2, 3};
    ASSERT_EQ(1, vec[0]);
    ASSERT_EQ(2, vec[1]);
    ASSERT_EQ(3, vec[2]);
}
TEST(RelativeVectorTest, PushingBack) {
    RelativeVector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    ASSERT_EQ(1, vec[0]);
    ASSERT_EQ(2, vec[1]);
    ASSERT_EQ(3, vec[2]);
}
TEST(RelativeVectorTest, Iteration) {
    RelativeVector<int> vec = {1, 2, 3, 4};
    std::array<int, 4> exp = {1, 2, 3, 4};
    int i = 0;
    for(auto it = vec.begin(); it != vec.end(); it++) {
        ASSERT_EQ(exp[i++], *it);
    }
    i = 0;
    for(auto it = vec.cbegin(); it != vec.cend(); it++) {
        ASSERT_EQ(exp[i++], *it);
    }
}
TEST(RelativeVectorTest, Insertion) {
    RelativeVector<int> vec = {1, 2, 4, 5};
    auto itr = std::find(vec.begin(), vec.end(), 4);
    vec.insert(itr, 3);
    ASSERT_EQ(vec[0], 1);
    ASSERT_EQ(vec[1], 2);
    ASSERT_EQ(vec[2], 3);
    ASSERT_EQ(vec[3], 4);
    ASSERT_EQ(vec[4], 5);
}
TEST(RelativeVectorTest, Eraseing) {
    RelativeVector<int> vec = {1, 2, 4, 3};
    auto itr = std::find(vec.begin(), vec.end(), 4);
    vec.erase(itr);
    ASSERT_EQ(vec[0], 1);
    ASSERT_EQ(vec[1], 2);
    ASSERT_EQ(vec[2], 3);
}
TEST(RelativeVectorTest, CoolIteration) {
    RelativeVector<int> vec = {1, 2, 3, 4};
    int sum = 0;
    for(auto v : vec) {
        sum += v;
    }
    ASSERT_EQ(10, sum);
}
TEST(RelativeVectorTest, Downsizing) {
    RelativeVector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    vec.resize(4);
    int sum = 0;
    for(auto v : vec) {
        sum += v;
    }
    ASSERT_EQ(10, sum);
    ASSERT_THROW(vec[5], std::out_of_range);
}
TEST(RelativeVectorTest, UpSizing) {
    RelativeVector<int> vec = {1, 2, 3, 4};
    vec.resize(8);
    ASSERT_EQ(8, vec.size());
    ASSERT_NO_THROW(vec[6]);
}
TEST(RelativeVectorTest, STDVectorIntegration) {
    std::vector<int> vec = {1, 2, 3, 4};
    RelativeVector<int> rel_vec = vec;
    ASSERT_EQ(4, rel_vec.size());
    ASSERT_EQ(1, rel_vec[0]);
    ASSERT_EQ(2, rel_vec[1]);
    ASSERT_EQ(3, rel_vec[2]);
    ASSERT_EQ(4, rel_vec[3]);
}
