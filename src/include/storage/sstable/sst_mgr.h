// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <storage/interface.h>
#include <storage/sstable/sst_mgr.h>
#include <storage/sstable/sstable.h>
#include <storage/sstable/flusher.h>
#include <vector>
#include <string>
#include <cstring>
#include <mutex>
#include <util/status.h>

namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;
using namespace storage;

class SSTMgr {
 public:
  SSTMgr(const std::string dir) : init_(false), dir_(dir), id_(0) {}
  virtual ~SSTMgr() {}
  Status Init();
  Status Recover(const std::vector<std::string> &files);
  Status Stop();
  Status Flush(Iterator &iter, const Log& log);
 public:
  const std::string SST_PREFIX = std::string("CtgSST");
 private:
  std::string nextFileName();
  Status getFilesFromDir_(std::vector<std::string> &files);
 private:
  bool init_;
  std::string dir_;
  std::mutex ssts_mu_;
  std::vector<SStable> ssts_;
  Flusher *flusher_;
  std::mutex id_mu_;
  uint64_t id_;
};

} // sstable
} // ctgfs
