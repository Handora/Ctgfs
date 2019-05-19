// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <wal/wal_protocol.h>
#include <vector>

namespace ctgfs {
namespace wal {

using util::Status;
using namespace util;
using namespace std;

struct Log {
 public:
  Log(WalType type, const std::string& key, const std::string& value)
    : type_(type), key_(key), value_(value) {}
  Log() : type_(UNKNOWN), key_(""), value_("") {}
  ~Log() {}
  WalType type_;
  std::string key_;
  std::string value_;
};

ostream& operator<<(ostream& out, const Log& s);
bool operator==(const Log &left, const Log &right);
istream& operator>>(istream& in, Log& s);

class Wal {
 public:
  Wal();
  ~Wal();
  Status Init(const std::string& path);
  Status Start();
  Status Recover();
  Status Stop();
  Status LogToDisk(const Log& log);
  Status AppendToMLog(const Log& log);
  Status SyncToDLog();
 public:
  // test only
  std::vector<Log> &GetMLog() { return mlog_; }
 private:
  Status LogToDiskWithoutLock(const Log& log);
 private:
  std::vector<Log> mlog_;
  ofstream dlog_;
  std::string log_file_;
  std::mutex log_mu_;
};

}  // namespace wal
}  // namespace ctgfs
