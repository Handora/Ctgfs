#include <master/heart_beat.h>
#include <master/heart_beat_detector.h>
#include <cassert>

namespace ctgfs {
namespace heart_beat {

void HeartBeatDetector::SendHeartBeat(
    ::google::protobuf::RpcController* controller,
    const ::ctgfs::HeartBeatMessageRequest* request,
    ::ctgfs::HeartBeatMessageResponse* reponse,
    ::google::protobuf::Closure* done) {
  brpc::ClosureGuard done_guard(done);
  // empty resp  needn't fill
  // use req to generate struct HeartBeatInfo
  // call the callback of master to update kv's info
  std::shared_ptr<HeartBeatInfo> info_ptr = std::make_shared<HeartBeatInfo>();
  (info_ptr->addr) = (request->addr());
  (info_ptr->type) = (request->type());
  master_ptr_->UpdateKVInfo(info_ptr);
}

}  // namespace heart_beat
}  // namespace ctgfs
