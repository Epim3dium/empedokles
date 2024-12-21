#include "templates/relative_string.hpp"
#include <gtest/gtest.h>
#include <cstring>

using namespace emp;
TEST(RelativeString, Initialization) {
    RelativeString str = "Hello, world!";
    ASSERT_EQ('!', str[str.size() - 1]);
    ASSERT_EQ('H', str[0]);
    ASSERT_EQ(',', str[5]);
    ASSERT_EQ(13, str.size());
}
TEST(RelativeString, Appending) {
    RelativeString str1 = "Hello";
    RelativeString str2 = ", world!";
    RelativeString str = str1 + str2;
    ASSERT_EQ('!', str[str.size() - 1]);
    ASSERT_EQ('H', str[0]);
    ASSERT_EQ(',', str[5]);
    ASSERT_EQ(13, str.size());
}
TEST(RelativeString, AppendingSingle) {
    const char* c_str = ", world!";
    RelativeString str = "Hello";
    for(int i = 0; i < strlen(c_str); i++) {
        str += c_str[i];
    }
    ASSERT_EQ('!', str[str.size() - 1]);
    ASSERT_EQ('H', str[0]);
    ASSERT_EQ(',', str[5]);
    ASSERT_EQ(13, str.size());
}
TEST(RelativeString, FindingChar) {
    RelativeString str = "Hello, world!";
    ASSERT_EQ(5, str.find(','));
    ASSERT_EQ(12, str.find('!'));
    ASSERT_EQ(0, str.find('H'));
    ASSERT_EQ(4, str.find('o'));
    ASSERT_EQ(RelativeString::npos, str.find('x'));
}
TEST(RelativeString, FindingSubstr) {
    RelativeString str = "Hello, world!";
    ASSERT_EQ(0, str.find(std::string("Hello")));
    ASSERT_EQ(7, str.find("world"));
    ASSERT_EQ(8, str.find("or"));
    ASSERT_EQ(12, str.find(RelativeString("!")));
    ASSERT_EQ(RelativeString::npos, str.find("boo"));
}
TEST(RelativeString, Substringing) {
    RelativeString str = "Hello, world!";
    ASSERT_EQ("Hello", str.substr(0, 5));
}
