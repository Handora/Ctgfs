#include "util/lru.h"
#include <gtest/gtest.h>
#include <iostream>

namespace ctgfs {
namespace util {

TEST(UtilTest, LRUCache) {
  LRUCache<int, int> lru(std::size_t(2));
  int value;
  EXPECT_EQ(true, lru.Put(1, 1));
  EXPECT_EQ(true, lru.Put(2, 2));
  EXPECT_EQ(true, lru.Get(1, value));       // returns 1
  EXPECT_EQ(1, value);       // returns 1
  EXPECT_EQ(true, lru.Put(3, 3));    // evicts key 2
  EXPECT_EQ(false, lru.Get(2, value));       // returns -1 (not found)
  EXPECT_EQ(true, lru.Put(4, 4));    // evicts key 1
  EXPECT_EQ(false, lru.Get(1, value));       // returns -1 (not found)
  EXPECT_EQ(true, lru.Get(3, value));       // returns 3
  EXPECT_EQ(3, value);       
  EXPECT_EQ(true, lru.Get(4, value));       // returns 4
  EXPECT_EQ(4, value);       
}

void print(const int& k, const int& v) {
  std::cout << "(" << k << ", " << v << ") is being deleted." << std::endl;
}

TEST(UtilTest, LRUCallback) {
  LRUCache<int, int> lru(std::size_t(2), &print);
  int value;
  EXPECT_EQ(true, lru.Put(1, 1));
  EXPECT_EQ(true, lru.Put(2, 2));
  EXPECT_EQ(true, lru.Get(1, value));       // returns 1
  EXPECT_EQ(1, value);       // returns 1
  EXPECT_EQ(true, lru.Put(3, 3));    // evicts key 2
  EXPECT_EQ(false, lru.Get(2, value));       // returns -1 (not found)
  EXPECT_EQ(true, lru.Put(4, 4));    // evicts key 1
  EXPECT_EQ(false, lru.Get(1, value));       // returns -1 (not found)
  EXPECT_EQ(true, lru.Get(3, value));       // returns 3
  EXPECT_EQ(3, value);       
  EXPECT_EQ(true, lru.Get(4, value));       // returns 4
  EXPECT_EQ(4, value);       
}

}}

