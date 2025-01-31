#include "../include/vv3.hpp"
#include <gtest/gtest.h>
#include <string>

struct Tracker {
  static int constructions;
  static int destructions;

  Tracker() { ++constructions; }
  Tracker(const Tracker&) { ++constructions; }
  Tracker(Tracker&&) noexcept { ++constructions; }
  ~Tracker() { ++destructions; }

  static void reset() {
    constructions = 0;
    destructions = 0;
  }
};

int Tracker::constructions = 0;
int Tracker::destructions = 0;

using vv3::Element;
using vv3::vector;

TEST(VectorTest, DefaultConstructor) {
  vector<int, std::string, double> vec;
  EXPECT_EQ(vec.size(), 0u);
}

TEST(VectorTest, PushBackDifferentTypes) {
  vector<int, std::string, double> vec;

  int a = 42;
  vec.push_back(a);
  EXPECT_EQ(vec.size(), 1u);
  EXPECT_EQ(vec.get<int>(0), 42);

  std::string s = "hello";
  vec.push_back(s);
  EXPECT_EQ(vec.size(), 2u);
  EXPECT_EQ(vec.get<std::string>(1), "hello");

  double d = 3.14;
  vec.push_back(d);
  EXPECT_EQ(vec.size(), 3u);
  EXPECT_DOUBLE_EQ(vec.get<double>(2), 3.14);
}

TEST(VectorTest, AccessElementsOperator) {
  vector<int, std::string> vec;
  vec.push_back(100);
  std::string hello = "world";
  vec.push_back(hello);

  Element e0 = vec[0];
  EXPECT_EQ(e0.type_index, 0u);
  EXPECT_EQ(*reinterpret_cast<int*>(e0.data), 100);

  Element e1 = vec[1];
  EXPECT_EQ(e1.type_index, 1u);
  EXPECT_EQ(*reinterpret_cast<std::string*>(e1.data), "world");
}

TEST(VectorTest, CopyConstructor) {
  vector<int, std::string> vec1;
  vec1.push_back(10);
  vec1.push_back(std::string("copy"));

  vector<int, std::string> vec2 = vec1;
  EXPECT_EQ(vec2.size(), 2u);
  EXPECT_EQ(vec2.get<int>(0), 10);
  EXPECT_EQ(vec2.get<std::string>(1), "copy");
}

TEST(VectorTest, MoveConstructor) {
  vector<int, std::string> vec1;
  vec1.push_back(20);
  vec1.push_back(std::string("move"));

  vector<int, std::string> vec2 = std::move(vec1);
  EXPECT_EQ(vec2.size(), 2u);
  EXPECT_EQ(vec2.get<int>(0), 20);
  EXPECT_EQ(vec2.get<std::string>(1), "move");

  EXPECT_EQ(vec1.size(), 0u);
}

TEST(VectorTest, CopyAssignment) {
  vector<int, std::string> vec1;
  vec1.push_back(30);
  vec1.push_back(std::string("assign"));

  vector<int, std::string> vec2;
  vec2 = vec1;
  EXPECT_EQ(vec2.size(), 2u);
  EXPECT_EQ(vec2.get<int>(0), 30);
  EXPECT_EQ(vec2.get<std::string>(1), "assign");
}

TEST(VectorTest, MoveAssignment) {
  vector<int, std::string> vec1;
  vec1.push_back(40);
  vec1.push_back(std::string("move_assign"));

  vector<int, std::string> vec2;
  vec2 = std::move(vec1);
  EXPECT_EQ(vec2.size(), 2u);
  EXPECT_EQ(vec2.get<int>(0), 40);
  EXPECT_EQ(vec2.get<std::string>(1), "move_assign");

  EXPECT_EQ(vec1.size(), 0u);
}

TEST(VectorTest, ExceptionOnWrongTypeAccess) {
  vector<int, std::string> vec;
  vec.push_back(50);
  vec.push_back(std::string("test"));

  EXPECT_NO_THROW(vec.get<int>(0));
  EXPECT_NO_THROW(vec.get<std::string>(1));

  EXPECT_THROW(vec.get<std::string>(0), std::bad_cast);
  EXPECT_THROW(vec.get<int>(1), std::bad_cast);
}

TEST(VectorTest, PushBackRvalueReferences) {
  vector<std::string> vec;
  vec.push_back("temporary");

  EXPECT_EQ(vec.size(), 1u);
  EXPECT_EQ(vec.get<std::string>(0), "temporary");

  std::string s = "lvalue";
  vec.push_back(s);

  EXPECT_EQ(vec.size(), 2u);
  EXPECT_EQ(vec.get<std::string>(1), "lvalue");
}

TEST(VectorTest, MultiplePushBacks) {
  vector<int, std::string, double> vec;
  const int num_elements = 100;

  for (int i = 0; i < num_elements; ++i) {
    if (i % 3 == 0) {
      vec.push_back(i);
    } else if (i % 3 == 1) {
      vec.push_back(std::string("str" + std::to_string(i)));
    } else {
      vec.push_back(static_cast<double>(i) * 1.1);
    }
  }

  EXPECT_EQ(vec.size(), num_elements);

  for (int i = 0; i < num_elements; ++i) {
    if (i % 3 == 0) {
      EXPECT_EQ(vec.get<int>(i), i);
    } else if (i % 3 == 1) {
      EXPECT_EQ(vec.get<std::string>(i), "str" + std::to_string(i));
    } else {
      EXPECT_DOUBLE_EQ(vec.get<double>(i), static_cast<double>(i) * 1.1);
    }
  }
}

TEST(VectorTest, DestructionOfElements) {
  Tracker::reset();
  {
    vector<Tracker> vec;
    vec.push_back(Tracker());
    vec.push_back(Tracker());
    EXPECT_EQ(Tracker::constructions, 2);
    EXPECT_EQ(Tracker::destructions, 0);
  }
  EXPECT_EQ(Tracker::destructions, 2);
}

TEST(VectorTest, AccessOutOfBounds) {
  vector<int, std::string> vec;
  vec.push_back(60);
  vec.push_back(std::string("out_of_bounds"));

  EXPECT_NO_THROW(vec.get<int>(0));
  EXPECT_NO_THROW(vec.get<std::string>(1));

  EXPECT_THROW(vec.get<int>(2), std::bad_cast);
}

TEST(VectorTest, EmptyVectorAccess) {
  vector<int, std::string> vec;
  EXPECT_EQ(vec.size(), 0u);

  EXPECT_THROW(vec.get<int>(0), std::bad_cast);
}

TEST(VectorTest, ReserveEntries) {
  vector<int, std::string> vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);

  EXPECT_EQ(vec.size(), 3u);

  for (int i = 4; i <= 100; ++i) {
    vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), 100u);
  EXPECT_EQ(vec.get<int>(99), 100);
}

TEST(VectorTest, HeterogeneousComplexTypes) {
  struct Complex {
    std::string name;
    int value;

    bool operator==(const Complex& other) const {
      return name == other.name && value == other.value;
    }
  };

  vector<int, std::string, Complex> vec;

  vec.push_back(7);
  vec.push_back(std::string("complex"));
  Complex c1 = {"test", 42};
  vec.push_back(c1);

  EXPECT_EQ(vec.size(), 3u);
  EXPECT_EQ(vec.get<int>(0), 7);
  EXPECT_EQ(vec.get<std::string>(1), "complex");
  EXPECT_EQ(vec.get<Complex>(2), c1);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
