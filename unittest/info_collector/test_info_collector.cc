/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#include <gtest/gtest.h>
#include <fs/extent_server.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <thread>
#include "master/master.h"
#include "master/master_protocol.h"

TEST(COLLECTORTest, Basic) {
  int r;

  /* test the basic functionality of info_collector. */
  int count = 0;
  char *count_env = getenv("RPC_COUNT");
  if(count_env != NULL){
    count = atoi(count_env);
  }

  rpcs server(atoi("62232"), count);
  using ctgfs::master::Master;
  Master ms;
  server.reg(master_protocol::heart_beat, &ms, &Master::UpdateKVInfo);

  extent_server src;  
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