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
  LOG(INFO) << "in heart beat" << std::endl;
  // empty resp  needn't fill
  // use req to generate struct HeartBeatInfo
  // call the callback of master to update kv's info
  std::shared_ptr<HeartBeatInfo> info_ptr = std::make_shared<HeartBeatInfo>();
  LOG(INFO) << "give value to info_ptr->addr" << std::endl;
  (info_ptr->addr) = (request->addr());
  (info_ptr->type) = (request->type());
  LOG(INFO) << "updateKVInfo" << std::endl;
  master_ptr_->UpdateKVInfo(info_ptr);
  LOG(INFO) << "collect heart beat end" << std::endl;
}

}  // namespace heart_beat
}  // namespace ctgfs
