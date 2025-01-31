#include "../include/vv2.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace vv2;

TEST(VectorTest, InitiallyEmpty) {
  vector<int, double, std::string> vec;
  EXPECT_EQ(vec.size(), 0);
}

TEST(VectorTest, PushBackSingleType) {
  vector<int, double, std::string> vec;
  vec.push_back(42);
  EXPECT_EQ(vec.size(), 1);
  auto elem = vec[0];
  EXPECT_EQ(elem.type_index, 0);
  EXPECT_EQ(vec.get<int>(0), 42);
}

TEST(VectorTest, PushBackMultipleTypes) {
  vector<int, double, std::string> vec;
  vec.push_back(42);
  vec.push_back(3.14);
  vec.push_back(std::string("hello"));
  ASSERT_EQ(vec.size(), 3);
  auto elem0 = vec[0];
  ASSERT_EQ(elem0.type_index, 0);
  ASSERT_EQ(*reinterpret_cast<int *>(elem0.data), 42);
  auto elem1 = vec[1];
  ASSERT_EQ(elem1.type_index, 1);
  ASSERT_DOUBLE_EQ(*reinterpret_cast<double *>(elem1.data), 3.14);
  auto elem2 = vec[2];
  ASSERT_EQ(elem2.type_index, 2);
  ASSERT_EQ(*reinterpret_cast<std::string *>(elem2.data), "hello");
}

TEST(VectorTest, GetCorrectType) {
  vector<int, double, std::string> vec;
  vec.push_back(42);
  vec.push_back(3.14);
  vec.push_back(std::string("hello"));
  ASSERT_EQ(vec.get<int>(0), 42);
  ASSERT_DOUBLE_EQ(vec.get<double>(1), 3.14);
  ASSERT_EQ(vec.get<std::string>(2), "hello");
}

TEST(VectorTest, GetIncorrectTypeThrows) {
  vector<int, double, std::string> vec;
  vec.push_back(42);
  vec.push_back(3.14);
  vec.push_back(std::string("hello"));
  ASSERT_THROW(vec.get<double>(0), std::bad_cast);
  ASSERT_THROW(vec.get<std::string>(1), std::bad_cast);
  ASSERT_THROW(vec.get<int>(2), std::bad_cast);
}

TEST(VectorTest, MultiplePushBacks) {
  vector<int, double, std::string> vec;
  for (int i = 0; i < 10; ++i) {
    vec.push_back(i);
  }
  ASSERT_EQ(vec.size(), 10);
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQ(vec.get<int>(i), i);
  }
}

TEST(VectorTest, ReserveEntries) {
  vector<int, double, std::string> vec;
  for (int i = 0; i < 100; ++i) {
    vec.push_back(i);
  }
  ASSERT_EQ(vec.size(), 100);
  for (int i = 0; i < 100; ++i) {
    ASSERT_EQ(vec.get<int>(i), i);
  }
}

TEST(VectorTest, MixedTypes) {
  vector<int, double, std::string> vec;
  vec.push_back(1);
  vec.push_back(2.5);
  vec.push_back(std::string("test"));
  vec.push_back(3);
  vec.push_back(4.5);
  ASSERT_EQ(vec.size(), 5);
  ASSERT_EQ(vec.get<int>(0), 1);
  ASSERT_DOUBLE_EQ(vec.get<double>(1), 2.5);
  ASSERT_EQ(vec.get<std::string>(2), "test");
  ASSERT_EQ(vec.get<int>(3), 3);
  ASSERT_DOUBLE_EQ(vec.get<double>(4), 4.5);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

