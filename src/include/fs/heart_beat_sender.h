/*
 * author: OneDay_(ltang970618@gmail.com)
 **/
#pragma once
#include <brpc/channel.h>
#include <butil/time.h>
#include <gflags/gflags.h>
#include <master.pb.h>
#include <util/status.h>
#include <memory>
#include <string>

namespace ctgfs {
namespace heart_beat {
// fs should fill this struct
// and use this struct to construct heart_beat_sender
// the info is as same as heart_beat_proto
struct HeartBeatInfo {
  HeartBeatMessageRequest_HeartBeatType type;
  std::string addr;
  unsigned int file_num;
  unsigned int disk_usage;
  // assume deep copy to make data persistent
  //  void operator=(const HeartBeatInfo& heart_beat_info) {
  //    addr = heart_beat_info.addr;
  //  }
};
// fs will get this struct
// if he call the get method
// the info is as same as heart_beat_proto
struct HeartBeatResp {};

// fs should create a sender to send heart beat package to master
// call sendHeartBeat method to send a package
class HeartBeatSender {
 public:
  // master addr
  // addr : ip:port
  HeartBeatSender(const std::string& addr,
                  const std::shared_ptr<HeartBeatInfo>);
  // you can change the heart beat info before send package
  void SetHeartBeatInfo(const std::shared_ptr<HeartBeatInfo>);
  // according heartbeatinfo send a req
  util::Status SendHeartBeat();
  // I'm sure if this can work so you'd better not call this function
  // because the resp maybe async you will get a null ptr or wild ptr
  HeartBeatResp* GetHeartBeatResponse();

 private:
  // should be init every time before connect
  brpc::Channel heart_beat_channel_;
  std::string addr_;
  // cur req
  std::shared_ptr<HeartBeatMessageRequest> heart_beat_message_req_;
  // cur res
  std::shared_ptr<HeartBeatMessageResponse> heart_beat_message_res_;
  void initChannel(const std::string& addr);
};

}  // namespace heart_beat
}  // namespace ctgfs
