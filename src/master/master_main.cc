/*
 * author: fftlover(ltang970618@gmail.com)
**/

#include <fs/heart_beat_sender.h>
#include <master/master.h>
#include <brpc/server.h>

int main() {
  brpc::Server server;
  ::ctgfs::master::Master master;
  server.AddService(&master, brpc::SERVER_DOESNT_OWN_SERVICE);
  brpc::ServerOptions options;
  std::string addr = std::string("127.0.0.1:1234");
  server.Start(addr.c_str(), &options);
  server.RunUntilAskedToQuit();
}
