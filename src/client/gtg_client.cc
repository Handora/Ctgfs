// Author: Chen Qian(qcdsr970209@gmail.com)
// gtg client implementation.

#include <arpa/inet.h>
#include <client/gtg_client.h>
#include <master/gtg.h>
#include "rpc/rpc.h"

#include <stdio.h>
#include <iostream>
#include <sstream>

namespace ctgfs {
namespace client {

GTC::GTC(std::string dst) {
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl_ = new rpcc(dstsock);
  if (cl_->bind() < 0) {
    printf("GTC: call bind error\n");
  }
}

int GTC::get_ts(int &ts) {
  return cl_->call(master::GTG::GET_TS, 0 /*useless*/, ts);
}

}  // namespace client
}  // namespace ctgfs
