#include <gtest/gtest.h>
#include "io/serializer.hpp"

using namespace emp;
TEST(SerializerTest, WriteReadLiterals) {
    Glob glob;
    float var1 = 2137.f;
    int var2 = 420;
    float var3 = 69.f;
    int var4 = -1;
    vec2f var5 = {9.f, 11.f};
    uint32_t var6 = 2016;

    glob.encode(var1);
    glob.encode(var2);
    glob.encode(var3);
    glob.encode(var4);
    glob.encode(var5);
    glob.encode(var6);

    float var1read, var3read;
    int var2read, var4read;
    vec2f var5read;
    uint32_t var6read;

    glob.decode(var1read);
    glob.decode(var2read);
    glob.decode(var3read);
    glob.decode(var4read);
    glob.decode(var5read);
    glob.decode(var6read);

    ASSERT_EQ(var1, var1read);
    ASSERT_EQ(var2, var2read);
    ASSERT_EQ(var3, var3read);
    ASSERT_EQ(var4, var4read);
    ASSERT_EQ(var5, var5read);
    ASSERT_EQ(var6, var6read);
}
TEST(SerializerTest, WriteReadArrays) {
    Glob glob;
    std::array<int, 4> arr = {1, 2, 3, 4};
    glob.encode(arr);

    std::array<int, 4> arr_read;
    glob.decode(arr_read);
    for(int i = 0; i < arr.size(); i++) {
        ASSERT_EQ(arr[i], arr_read[i]);
    }
}
TEST(SerializerTest, WriteReadVectors) {
    Glob glob;
    std::vector<int> vec = {1, 2, 3, 4};
    glob.encode(vec);
    
    std::vector<int> vec_read;
    glob.decode(vec_read);
    for(int i = 0; i < vec.size(); i++) {
        ASSERT_EQ(vec[i], vec_read[i]);
    }
}
TEST(SerializerTest, WriteReadString) {
    Glob glob;
    std::string str = "Hello, world!";
    glob.encode(str);
    
    std::string str_read;
    glob.decode(str_read);
    ASSERT_EQ(str, str_read);
}
