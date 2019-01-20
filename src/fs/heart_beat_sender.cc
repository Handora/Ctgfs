/*
 * author: OneDay_(ltang970618@gmail.com)
 **/
#include <fs/heart_beat_sender.h>

namespace ctgfs {
namespace heart_beat {
HeartBeatSender::HeartBeatSender(
    const std::string& addr,
    const std::shared_ptr<HeartBeatInfo> heart_beat_message_req)
    : addr_(addr) {
  heart_beat_message_req_ = std::make_shared<HeartBeatMessageRequest>();
  heart_beat_message_res_ = std::make_shared<HeartBeatMessageResponse>();
  SetHeartBeatInfo(heart_beat_message_req);
}

void HeartBeatSender::SetHeartBeatInfo(
    const std::shared_ptr<HeartBeatInfo> heart_beat_info) {
  auto req_ptr = heart_beat_message_req_.get();
  auto info_ptr = heart_beat_info.get();
  req_ptr->set_type(info_ptr->type);
  req_ptr->set_addr(info_ptr->addr);
}

util::Status HeartBeatSender::SendHeartBeat() {
  initChannel(addr_);
  MasterService_Stub stub(&heart_beat_channel_);
  brpc::Controller ctrl;
  stub.SendHeartBeat(&ctrl, heart_beat_message_req_.get(),
                     heart_beat_message_res_.get(), NULL);
  if (ctrl.Failed()) {
    LOG(ERROR) << "Error Msg: " << ctrl.ErrorText() << std::endl;
    return util::Status::HeartBeatFail();
  }

  return util::Status::OK();
}

void HeartBeatSender::initChannel(const std::string& addr) {
  brpc::ChannelOptions options;
  heart_beat_channel_.Init(addr.c_str(), &options);
}

}  // namespace heart_beat
}  // namespace ctgfs
