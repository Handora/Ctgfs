/*
* author: fftlover(ltang970618@gmail.com)
**/
#include <brpc/server.h>
#include <butil/logging.h>
#include <master.pb.h>
#include <fs/fs_service.h>
#include <gflags/gflags.h>
#include <memory>

namespace ctgfs {
namespace fs {

void AbstractFSService::DoCommandOnFS(
    ::google::protobuf::RpcController* controller,
    const ::ctgfs::ClientKVRequest* request,
    ::ctgfs::FileSystemResponse* response, ::google::protobuf::Closure* done) {
  brpc::ClosureGuard done_guard(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  if (!solveHeader(request)) {
    cntl->SetFailed("FileSystem reject request");
    return;
  }
  if (cntl->has_remote_stream()) {
    solveFileStream(cntl, response);
  } else {
    solveFileWithoutStream(cntl, response);
  }
}

void AbstractFSService::solveFileStream(brpc::Controller* cntl,
                                        ::ctgfs::FileSystemResponse* resp) {
  brpc::StreamOptions stream_options;
  stream_options.handler = receiver_ptr_.get();
  if (brpc::StreamAccept(&sd_, *cntl, &stream_options) != 0) {
    cntl->SetFailed("Fail to accept stream");
    return;
  }
}

void AbstractFSService::solveFileWithoutStream(
    brpc::Controller* cntl, ::ctgfs::FileSystemResponse* resp) {}

}  // fs
}  // ctgfs
