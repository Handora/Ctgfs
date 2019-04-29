/*
 * author: fftlover(ltang970618@gmail.com)
**/

#include <fs/heart_beat_sender.h>
#include <master/master.h>

using namespace ctgfs::util;
using namespace ctgfs::heart_beat;
using namespace ctgfs::master;
using namespace ctgfs;

int main() {
  std::string fs_addr = std::string("57555");
  auto heart_beat_info = std::make_shared<HeartBeatInfo>();
  heart_beat_info->type = HeartBeatMessageRequest_HeartBeatType::
    HeartBeatMessageRequest_HeartBeatType_kRegist;
  heart_beat_info->addr = fs_addr;
  std::string addr = std::string("127.0.0.1:1234");
  HeartBeatSender sender(addr, heart_beat_info);
  auto send_status = sender.SendHeartBeat();
}
