// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <fs/extent_server.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

TEST(MOVETest, Move) {

  int count = 0;
  char* count_env = getenv("RPC_COUNT");
  if (count_env != nullptr) {
    count = atoi(count_env);
  }

  /* start rpc server. */
  rpcs server(atoi("57123"), count);
  extent_server dst; 

  server.reg(extent_protocol::get, &dst, &extent_server::get);
  server.reg(extent_protocol::getattr, &dst, &extent_server::getattr);
  server.reg(extent_protocol::put, &dst, &extent_server::put);
  server.reg(extent_protocol::remove, &dst, &extent_server::remove);
  server.reg(extent_protocol::setattr, &dst, &extent_server::setattr);

  extent_server src; 
  int r;
  /* insert fake data */
  EXPECT_EQ(extent_protocol::OK, src.put(1, "content_1", r));
  EXPECT_EQ(extent_protocol::OK, src.put(2, "content_2", r));
  EXPECT_EQ(extent_protocol::OK, src.put(3, "content_3", r));

  /* check  */
  std::string content;
  EXPECT_EQ(extent_protocol::OK, src.get(1, content));
  EXPECT_EQ("content_1", content);
  EXPECT_EQ(extent_protocol::OK, src.get(2, content));
  EXPECT_EQ("content_2", content);
  EXPECT_EQ(extent_protocol::OK, src.get(3, content));
  EXPECT_EQ("content_3", content);

  std::vector<extent_protocol::extentid_t> to_be_moved{1, 2};

  /* test move semantics */
  EXPECT_EQ(extent_protocol::OK, src.move(to_be_moved, "127.0.0.1:57123"));

  /* check */
  EXPECT_EQ(extent_protocol::OK, dst.get(1, content));
  EXPECT_EQ("content_1", content);
  EXPECT_EQ(extent_protocol::OK, dst.get(2, content));
  EXPECT_EQ("content_2", content);

  EXPECT_EQ(extent_protocol::NOENT, src.get(1, content));
  EXPECT_EQ(extent_protocol::NOENT, src.get(2, content));
}

