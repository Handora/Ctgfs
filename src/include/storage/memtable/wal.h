// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <storage/log.h>
#include <vector>

namespace ctgfs {
namespace memtable {

using util::Status;
using namespace util;
using namespace std;
using namespace storage;

class Wal {
 public:
  Wal();
  ~Wal();
  Status Init(const std::string& dir, const std::function<void(const Log&)>& f);
  Status Recover();
  Status Start();
  Status Stop();
  Status LogToDisk(Log& log);
  Status LogCurrent(uint64_t offset);
  Status LogCurrent();
  uint64_t GetDiskLogSize();
 private:
  Status LogToDiskWithoutLock(Log& log);
 private:
  bool init_;
  int dlog_;
  std::string dir_;
  std::string filename_;
  std::string current_;
  std::mutex log_mu_;
  std::function<void(const Log&)> func_;
};

}  // namespace wal
}  // namespace ctgfs
