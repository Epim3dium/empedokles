#include "gtest/gtest.h"
#include "memory/relative_pointer.hpp"

using namespace emp;
TEST(RelativePointerTest, AssigningPointer) {
    RelativePointer<int> ptr = new int(5);
    ptr.set(new int(5));
    ASSERT_EQ(5, *ptr);
    delete ptr.get();
}
TEST(RelativePointerTest, AssigningPointerWhenRelativeIsMinus) {
    char buffer[sizeof(int) + sizeof(RelativePointer<int>)];
    //first allocating data
    auto* raw = new (buffer) int(2137);
    //then allocating relative pointer
    RelativePointer<int>* ptr = new(buffer + sizeof(int))RelativePointer<int>();
    ptr->set(raw);
    ASSERT_EQ(2137, **ptr);
}
TEST(RelativePointerTest, Comparison) {
    auto* raw = new int(6969);
    RelativePointer<int> ptr1, ptr2;
    ptr1.set(raw);
    ptr2.set(raw);
    ASSERT_NE(ptr1.offset, ptr2.offset);
    ASSERT_TRUE(ptr1.is_equal(ptr2));
    delete raw;
}
TEST(RelativePointerTest, Casting) {
    RelativePointer<int> ptr = new int(6969);
    int* raw = ptr;
    ASSERT_EQ(raw, ptr);
    ASSERT_EQ(*raw, *ptr);
    delete raw;
}

TEST(RelativePointerTest, IsNullTest) {
    RelativePointer<int> ptr = nullptr;
    ASSERT_EQ(ptr, nullptr);
    ASSERT_TRUE(ptr.is_null());
}
