#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "../include/vv2.hpp"

using VectorType = vv2::vector<int, double, std::string>;


//------------------------------------------------------------------------------
// Vector Tests
//------------------------------------------------------------------------------

TEST(VectorTest, DefaultConstruction) {
  std::cout <<"AAAAAAAAAAAAAa" << std::endl;
  vv2::vector2<int, short, double> v;
  std::cout << v.test() << std::endl;
}
//
// TEST(VectorTest, PushBackInt) {
//   VectorType vec;
//   vec.push_back(42);
//
//   EXPECT_EQ(0u, vec[0].index());
//   EXPECT_EQ(42, vec[0].get<int>());
// }
//
// TEST(VectorTest, PushBackString) {
//   VectorType vec;
//   vec.push_back(std::string("Hello Vector"));
//
//   EXPECT_EQ(2u, vec[0].index());
//   EXPECT_EQ("Hello Vector", vec[0].get<std::string>());
// }
//
// TEST(VectorTest, MultiplePushBack) {
//   VectorType vec;
//   vec.push_back(10);
//   vec.push_back(3.14);
//   vec.push_back(std::string("Test"));
//
//   EXPECT_EQ(0u, vec[0].index());
//   EXPECT_EQ(10, vec[0].get<int>());
//
//   EXPECT_EQ(1u, vec[1].index());
//   EXPECT_DOUBLE_EQ(3.14, vec[1].get<double>());
//
//   EXPECT_EQ(2u, vec[2].index());
//   EXPECT_EQ("Test", vec[2].get<std::string>());
// }
//
// TEST(VectorTest, EmplaceBackString) {
//   VectorType vec;
//   vec.emblace_back(std::in_place_type<std::string>, "Emplaced");
//
//   EXPECT_EQ(2u, vec[0].index());
//   EXPECT_EQ("Emplaced", vec[0].get<std::string>());
// }
//
// TEST(VectorTest, CopyConstruction) {
//   VectorType vec;
//   vec.push_back(99);
//   vec.push_back(std::string("CopyMe"));
//
//   VectorType copy(vec);
//
//   EXPECT_EQ(0u, copy[0].index());
//   EXPECT_EQ(99, copy[0].get<int>());
//
//   EXPECT_EQ(2u, copy[1].index());
//   EXPECT_EQ("CopyMe", copy[1].get<std::string>());
// }
//
// TEST(VectorTest, MoveConstruction) {
//   VectorType vec;
//   vec.push_back(123);
//   vec.push_back(std::string("MoveMe"));
//
//   VectorType moved(std::move(vec));
//
//   EXPECT_EQ(0u, moved[0].index());
//   EXPECT_EQ(123, moved[0].get<int>());
//
//   EXPECT_EQ(2u, moved[1].index());
//   EXPECT_EQ("MoveMe", moved[1].get<std::string>());
// }
//
// TEST(VectorTest, CopyAssignment) {
//   VectorType vec1;
//   vec1.push_back(2020);
//   vec1.push_back(std::string("Year"));
//
//   VectorType vec2;
//   vec2.push_back(std::string("OldData"));
//
//   vec2 = vec1; 
//
//   EXPECT_EQ(0u, vec2[0].index());
//   EXPECT_EQ(2020, vec2[0].get<int>());
//
//   EXPECT_EQ(2u, vec2[1].index());
//   EXPECT_EQ("Year", vec2[1].get<std::string>());
// }
//
// TEST(VectorTest, MoveAssignment) {
//   VectorType vec1;
//   vec1.push_back(55);
//   vec1.push_back(std::string("MoveAssignment"));
//
//   VectorType vec2;
//   vec2.push_back(std::string("OldData"));
//
//   vec2 = std::move(vec1); 
//
//   EXPECT_EQ(0u, vec2[0].index());
//   EXPECT_EQ(55, vec2[0].get<int>());
//
//   EXPECT_EQ(2u, vec2[1].index());
//   EXPECT_EQ("MoveAssignment", vec2[1].get<std::string>());
// }
//
// TEST(VectorTest, ReserveNoCrash) {
//   VectorType vec;
//   vec.reserve(10);
//
//   for (int i = 0; i < 10; ++i) {
//     vec.push_back(i);
//   }
//   EXPECT_EQ(0u, vec[0].index());
//   EXPECT_EQ(0, vec[0].get<int>());
//
//   EXPECT_EQ(0u, vec[9].index());
//   EXPECT_EQ(9, vec[9].get<int>());
// }

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

