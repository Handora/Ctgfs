#include "rpc/rpc.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include "fs/extent_server.h"
#include <fs/info_collector.h>

// Main loop of extent server

using namespace ::ctgfs::server;

int
main(int argc, char *argv[])
{
  int count = 0;

  if(argc != 3){
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(1);
  }

  setvbuf(stdout, NULL, _IONBF, 0);

  char *count_env = getenv("RPC_COUNT");
  if(count_env != NULL){
    count = atoi(count_env);
  }

  rpcs server(atoi(argv[1]), count);
  extent_server ls;

  server.reg(extent_protocol::get, &ls, &extent_server::get);
  server.reg(extent_protocol::getattr, &ls, &extent_server::getattr);
  server.reg(extent_protocol::put, &ls, &extent_server::put);
  server.reg(extent_protocol::remove, &ls, &extent_server::remove);
  server.reg(extent_protocol::setattr, &ls, &extent_server::setattr);
  server.reg(extent_protocol::move, &ls, &extent_server::move);
  
  auto ins = ::ctgfs::info_collector::InfoCollector::GetInstance();
  ::ctgfs::info_collector::InfoCollector::KVInfo info;
  info.addr = std::string(argv[1]);
  info.sz = 10000;
  ins->Set(info);
  ins->Regist(argv[2]);
  while(1)
    sleep(1000);
}
