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
  Status Init(const std::string& dir);
  Status Recover();
  Status Start();
  Status Stop();
  Status LogToDisk(Log& log);
  Status AppendToMLog(const Log& log);
  Status SyncToDLog();
 private:
  Status LogToDiskWithoutLock(Log& log);
 private:
  bool init_;
  std::vector<Log> mlog_;
  int dlog_;
  std::string dir_;
  std::string filename_;
  std::mutex log_mu_;
};

}  // namespace wal
}  // namespace ctgfs
