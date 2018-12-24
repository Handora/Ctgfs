/*
* author: OneDay_(ltang970618@gmail.com)
**/
#pragma once
#include <brpc/channel.h>
#include <butil/time.h>
#include <heart_beat.pb.h>
#include <master/master.h>
#include <memory>

namespace ctgfs {
namespace heart_beat {
class HeartBeatDetector : public HeartBeatService {
 public:
  inline HeartBeatDetector(std::shared_ptr<master::Master> master_ptr)
      : master_ptr_(master_ptr) {}
  ~HeartBeatDetector() {}
  void SendHeartBeat(::google::protobuf::RpcController* controller,
                     const ::ctgfs::HeartBeatMessageRequest* request,
                     ::ctgfs::HeartBeatMessageResponse* response,
                     ::google::protobuf::Closure* done);

 private:
  std::shared_ptr<master::Master> master_ptr_;
};
}  // namespace heart_beat
}  // namespace ctgfs
