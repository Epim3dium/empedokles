#include <glm/ext/matrix_transform.hpp>
#include <gtest/gtest.h>
#include "io/serializer.hpp"
#include "math/math_defs.hpp"

using namespace emp;
TEST(SerializerTest, EncodeDecodeLiterals) {
    Glob glob;
    float var1 = 2137.f;
    int var2 = 420;
    float var3 = 69.f;
    int var4 = -1;
    vec2f var5 = {9.f, 11.f};
    uint32_t var6 = 2016;
    TransformMatrix var7 = glm::translate(glm::mat4(1), {5, 4, 0});

    glob.encode(var1);
    glob.encode(var2);
    glob.encode(var3);
    glob.encode(var4);
    glob.encode(var5);
    glob.encode(var6);
    glob.encode(var7);

    float var1read, var3read;
    int var2read, var4read;
    vec2f var5read;
    uint32_t var6read;
    TransformMatrix var7read;

    glob.decode(var1read);
    glob.decode(var2read);
    glob.decode(var3read);
    glob.decode(var4read);
    glob.decode(var5read);
    glob.decode(var6read);
    glob.decode(var7read);

    ASSERT_EQ(var1, var1read);
    ASSERT_EQ(var2, var2read);
    ASSERT_EQ(var3, var3read);
    ASSERT_EQ(var4, var4read);
    ASSERT_EQ(var5, var5read);
    ASSERT_EQ(var6, var6read);
    ASSERT_EQ(var7, var7read);
}
TEST(SerializerTest, EncodeDecodeMath) {
    Glob glob;
    glm::vec<3, float> v1 = {1, 2, 3}, v3;
    glm::vec<2, int> v2 = {4, 5}, v4;
    glob.encode(v1);
    glob.encode(v2);

    glob.decode(v3);
    glob.decode(v4);
    ASSERT_EQ(v1, v3);
    ASSERT_EQ(v2, v4);
}
TEST(SerializerTest, EncodeDecodeArrays) {
    Glob glob;
    std::array<int, 4> arr = {1, 2, 3, 4};
    glob.encode(arr);

    std::array<int, 4> arr_read = {-1};
    glob.decode(arr_read);
    for(int i = 0; i < arr.size(); i++) {
        ASSERT_EQ(arr[i], arr_read[i]);
    }
}
TEST(SerializerTest, EncodeDecodeVectors) {
    Glob glob;
    std::vector<int> vec = {1, 2, 3, 4};
    glob.encode(vec);
    
    std::vector<int> vec_read = {-1};
    glob.decode(vec_read);
    for(int i = 0; i < vec.size(); i++) {
        ASSERT_EQ(vec[i], vec_read[i]);
    }

    std::vector< std::vector< float > > vecVec
    {
        {1.f},
        {0.5f, 1.5f},
        {1.f, 1.f, 1.f},
        {0.25f, 1.f, 1.f, 1.75f}
    };
    glob.encode(vecVec);
    std::vector< std::vector< float > > vecVec_read {{-1}};
    glob.encode(vecVec);

    glob.decode(vecVec_read);
    for(auto vec : vecVec_read) {
        ASSERT_EQ(vec.size(), std::reduce(vec.begin(), vec.end()));
    }
}
TEST(SerializerTest, EncodeDecodeString) {
    Glob glob;
    std::string str = "Hello, world!";
    glob.encode(str);
    
    std::string str_read;
    glob.decode(str_read);
    ASSERT_EQ(str, str_read);
}
TEST(SerializerTest, EncodeDecodeMaps) {
    auto insertAndTest = [](auto map) {
        Glob glob;
        map["hello"] = 1;
        map["world"] = 2;
        map["!"] = 3;
        glob.encode(map);
        
        decltype(map) map_read = {{"h", -1}};
        glob.decode(map_read);
        ASSERT_EQ(map, map_read);
        ASSERT_EQ(3, map.at("!"));
    };
    
    insertAndTest(std::map<std::string, int>());
    insertAndTest(std::unordered_map<std::string, int>());
}
TEST(SerializerTest, EncodeDecodeSets) {
    auto insertAndTest = [](auto set) {
        Glob glob;
        set.insert(2);
        set.insert(3);
        set.insert(5);
        set.insert(7);
        glob.encode(set);
        
        decltype(set) set_read = {-1};
        glob.decode(set_read);
        ASSERT_EQ(set, set_read);
        ASSERT_TRUE(set.contains(5));
    };
    insertAndTest(std::set<int>());
    insertAndTest(std::unordered_set<int>());
}
struct TrivialStruct {
    float field1;
    int field2;
    char field3;
    double field4;
    short field5;
};
template<>
struct SerialConvert<TrivialStruct> {
    void encode(const TrivialStruct& var, IGlobWriter& writer) {
        writer.encode(var.field1);
        writer.encode(var.field2);
        writer.encode(var.field3);
        writer.encode(var.field4);
        writer.encode(var.field5);
    }
    void decode(TrivialStruct& var, IGlobReader& reader) {
        reader.decode(var.field1);
        reader.decode(var.field2);
        reader.decode(var.field3);
        reader.decode(var.field4);
        reader.decode(var.field5);
    }
};
TEST(SerializerTest, EncodeDecodeTrivial) {
    Glob glob;
    TrivialStruct data;
    data.field1 = 3.141f;
    data.field2 = 2137;
    data.field3 = 'x';
    data.field4 = 6.283;
    data.field5 = 0xff;
    glob.encode(data);
    TrivialStruct data_read;
    glob.decode(data_read);
    ASSERT_EQ(3.141f, data_read.field1);
    ASSERT_EQ(2137,   data_read.field2);
    ASSERT_EQ('x',    data_read.field3);
    ASSERT_EQ(6.283,  data_read.field4);
    ASSERT_EQ(0xff,   data_read.field5);
}
