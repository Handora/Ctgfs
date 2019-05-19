/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#include <gtest/gtest.h>
#include <fs/extent_server.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <master/master.h>
#include <master/lock_server.h>
#include <master/lock_server_cache.h>

using namespace ::ctgfs::server;
using namespace ::ctgfs::lock_server;

TEST(MOVETest, MoveInside) {

  int count = 0;
  char* count_env = getenv("RPC_COUNT");
  if (count_env != nullptr) {
    count = atoi(count_env);
  }

  /* start rpc server. */
  rpcs server(atoi("57123"), count);
  extent_server dst; 

  server.reg(extent_protocol::get, &dst, &extent_server::get);
  server.reg(extent_protocol::put, &dst, &extent_server::put);
  server.reg(extent_protocol::move, &dst, &extent_server::move);


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
  int rr = 0;
  EXPECT_EQ(extent_protocol::OK, src.move(std::move(to_be_moved), std::move("127.0.0.1:57123"), rr));
  // printf("r = %d\n", rr);

  /* check */
  EXPECT_EQ(extent_protocol::OK, dst.get(1, content));
  EXPECT_EQ("content_1", content);
  EXPECT_EQ(extent_protocol::OK, dst.get(2, content));
  EXPECT_EQ("content_2", content);

  EXPECT_EQ(extent_protocol::NOENT, src.get(1, content));
  EXPECT_EQ(extent_protocol::NOENT, src.get(2, content));
}


TEST(MOVETest, MasterCallMove) {

/* TEST FLOW: */
/* - preparation: */
/*   1. two rpc server for two extent_server, respectively */
/*   2. one rpc server for lock server */
/*   3. fake data generation */
/* - master::Move() */
/* - after Move check */

  int count = 0;
  char* count_env = getenv("RPC_COUNT");
  if (count_env != nullptr) {
    count = atoi(count_env);
  }

  /* 1.1 start rpc server for extent server (src), from where files should be moved out. */
  rpcs extent_src_rpcs(atoi("54123"), count);
  extent_server src; 
  extent_src_rpcs.reg(extent_protocol::move, &src, &extent_server::move);

  /* 1.2 start rpc server for extent server (dst), to where files should be moved. */
  rpcs extent_dst_rpcs(atoi("54234"), count);
  extent_server dst; 
  extent_dst_rpcs.reg(extent_protocol::put, &dst, &extent_server::put);

  /* 2. start rpc server for lock server */
  lock_server_cache ls;
  rpcs lock_server_rpcs(atoi("54345"), count);
  lock_server_rpcs.reg(lock_protocol::acquire, &ls, &lock_server_cache::acquire);

  /* 3. insert fake data */
  int r;
  EXPECT_EQ(extent_protocol::OK, src.put(1, "content_1", r));
  EXPECT_EQ(extent_protocol::OK, src.put(2, "content_2", r));
  EXPECT_EQ(extent_protocol::OK, src.put(3, "content_3", r));

  /* check before move */
  std::string content;
  EXPECT_EQ(extent_protocol::OK, src.get(1, content));
  EXPECT_EQ("content_1", content);
  EXPECT_EQ(extent_protocol::OK, src.get(2, content));
  EXPECT_EQ("content_2", content);
  EXPECT_EQ(extent_protocol::OK, src.get(3, content));
  EXPECT_EQ("content_3", content);


  /* master call move */
  using ctgfs::master::Master;
  Master master;
  master.Move(std::move("127.0.0.1:54345"), std::move(std::vector<unsigned long long>{1, 2}), std::move("127.0.0.1:54123"), std::move("127.0.0.1:54234"));

  /* check after move */
  EXPECT_EQ(extent_protocol::OK, dst.get(1, content));
  EXPECT_EQ("content_1", content);
  EXPECT_EQ(extent_protocol::OK, dst.get(2, content));
  EXPECT_EQ("content_2", content);

  EXPECT_EQ(extent_protocol::NOENT, src.get(1, content));
  EXPECT_EQ(extent_protocol::NOENT, src.get(2, content));
}

