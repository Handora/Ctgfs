// RPC stubs for clients to talk to extent_server

#include "client/extent_client.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

namespace ctgfs {
namespace client {

// The calls assume that the caller holds a lock on the extent

extent_client::extent_client(const std::string& dst) {
  // sockaddr_in dstsock;
  // make_sockaddr(dst.c_str(), &dstsock);
  // cl = new rpcc(dstsock);
  // if (cl->bind() != 0) {
  //   printf("extent_client: bind failed\n");
  // }
  cl = nullptr;
  ConnectTo(dst);
}

extent_client::~extent_client() {
  for (auto p : addr_to_rpcc_) {
    if (p.second) delete p.second;
  }
}

void extent_client::ConnectTo(const std::string& dst) {
  sockaddr_in dstsock;
  addr_ = dst;
  if (addr_to_rpcc_.find(dst) == addr_to_rpcc_.end()) {
    printf("create %s\n", dst.c_str());
    make_sockaddr(dst.c_str(), &dstsock);
    auto gen_cl = new rpcc(dstsock);
    if (gen_cl->bind() != 0) {
      printf("extent_client: bind failed\n");
    } else
      addr_to_rpcc_[dst] = gen_cl;
  }
  auto p = addr_to_rpcc_[dst];
  if (p) {
    cl = p;
  } else
    cl = nullptr;
}

std::string extent_client::GetCurAddr() { return addr_; }

extent_protocol::status extent_client::get(extent_protocol::extentid_t eid,
                                           std::string& buf) {
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::get, eid, buf);
  return ret;
}

extent_protocol::status extent_client::getattr(extent_protocol::extentid_t eid,
                                               extent_protocol::attr& attr) {
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::getattr, eid, attr);
  return ret;
}

extent_protocol::status extent_client::put(extent_protocol::extentid_t eid,
                                           std::string buf) {
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::put, eid, buf, r);
  return ret;
}

extent_protocol::status extent_client::remove(extent_protocol::extentid_t eid) {
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::remove, eid, r);
  return ret;
}

extent_protocol::status extent_client::setattr(extent_protocol::extentid_t eid,
                                               extent_protocol::attr& attr) {
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::setattr, eid, attr, r);
  return ret;
}

}  // client
}  // ctgfs
