// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <fs/extent_server.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <thread>
#include "master/master.h"
/* these should be fixed. */
#include "../master/lock_server.h"
#include "../master/lock_server_cache.h"

TEST(COLLECTORTest, Basic) {
  /* test the basic functionality of info_collector. */
  extent_server src; 
  int r;
  /* insert fake data */
  std::this_thread::sleep_for(std::chrono::seconds(5));
  EXPECT_EQ(extent_protocol::OK, src.put(11, "content_1", r));
  std::this_thread::sleep_for(std::chrono::seconds(5));
  EXPECT_EQ(extent_protocol::OK, src.put(21, "content_2", r));
  std::this_thread::sleep_for(std::chrono::seconds(5));
  EXPECT_EQ(extent_protocol::OK, src.put(31, "content_3", r));

  /* check  */
  std::string content;
  EXPECT_EQ(extent_protocol::OK, src.get(11, content));
  EXPECT_EQ("content_1", content);
  EXPECT_EQ(extent_protocol::OK, src.get(21, content));
  EXPECT_EQ("content_2", content);
  EXPECT_EQ(extent_protocol::OK, src.get(31, content));
  EXPECT_EQ("content_3", content);

  std::this_thread::sleep_for(std::chrono::seconds(5));
  EXPECT_EQ(extent_protocol::OK, src.remove(31, r));
  std::this_thread::sleep_for(std::chrono::seconds(5));
  EXPECT_EQ(extent_protocol::OK, src.remove(21, r));


  std::this_thread::sleep_for(std::chrono::seconds(30));
}