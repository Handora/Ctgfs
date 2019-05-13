/*
 * author: fftlover(ltang970618@gmail.com)
**/

#include "master/master.h"
#include "master/master_protocol.h"
#include "rpc/rpc.h"

using namespace ctgfs::master;

int
main(int argc, char *argv[])
{
  int count = 0;

  if(argc != 2){
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(1);
  }

  setvbuf(stdout, NULL, _IONBF, 0);

  char *count_env = getenv("RPC_COUNT");
  if(count_env != NULL){
    count = atoi(count_env);
  }

  rpcs server(atoi(argv[1]), count);

  Master ms;

  server.reg(master_protocol::heart_beat, &ms, &Master::UpdateKVInfo);
  server.reg(master_protocol::regist, &ms, &Master::Regist);

  while(1)
    sleep(1000);
}

