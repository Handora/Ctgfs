/*
 * author: fftlover(ltang970618@gmail.com)
 **/
#pragma once
#include <brpc/server.h>
#include <brpc/stream.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <client.pb.h>
#include <gflags/gflags.h>
#include <functional>
#include <iostream>
#include <memory>

namespace ctgfs {
namespace fs {

// handler of file stream
// fs should pass a ostringstream and callback
// when receive a file stream receiver will help write msg
// to ostringstream and call callback
// callback's type should be void();
class FileStreamReceiver : public brpc::StreamInputHandler {
 public:
  FileStreamReceiver(std::ostringstream& os, std::function<void()> callback)
      : os_(os), callback_(callback) {}
  ~FileStreamReceiver() = default;
  // ~FileStreamReceiver() {}
  virtual int on_received_messages(brpc::StreamId id,
                                   butil::IOBuf* const messages[],
                                   size_t size) {
    for (size_t i = 0; i < size; i++) {
      os_ << *messages[i];
    }
    callback_();
    return 0;
  }
  virtual void on_idle_timeout(brpc::StreamId id) {}

  virtual void on_closed(brpc::StreamId id) {}

 private:
  FileStreamReceiver() = delete;
  std::ostringstream& os_;
  std::function<void(void)> callback_;
};

// should pass ostringstream and callback void()
// abstract class you should rewrite solveHeader
// and you can rewrite solveFileStream and solveFileWithoutStream
// so that you can solve more situation
class AbstractFSService : public FileSystemService {
 public:
  AbstractFSService(std::ostringstream& os, std::function<void()> callback)
      : os_(os), callback_(callback), sd_(brpc::INVALID_STREAM_ID) {
    receiver_ptr_ =
        std::make_shared<FileStreamReceiver>(std::ref(os_), callback_);
  }
  ~AbstractFSService() {}
  void DoCommandOnFS(::google::protobuf::RpcController* controller,
                     const ::ctgfs::ClientKVRequest* request,
                     ::ctgfs::FileSystemResponse* response,
                     ::google::protobuf::Closure* done);

 protected:
  // solve kvreq return true if fs can receive this command else return false
  virtual bool solveHeader(const ::ctgfs::ClientKVRequest* request) = 0;
  // solve command with file stream like add file update file
  virtual void solveFileStream(brpc::Controller*, ::ctgfs::FileSystemResponse*);
  // solve command without file stream like rm file
  virtual void solveFileWithoutStream(brpc::Controller*,
                                      ::ctgfs::FileSystemResponse*);

 private:
  AbstractFSService() = delete;
  std::shared_ptr<FileStreamReceiver> receiver_ptr_;
  std::ostringstream& os_;
  std::function<void(void)> callback_;
  // set streamid=INVALID_STREAM_ID
  // because needn't write info
  brpc::StreamId sd_;
};

}  // fs
}  // ctgfs
