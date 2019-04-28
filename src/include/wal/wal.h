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

ostream& operator<<(ostream& out, const Log& s) {
  out << (int)(s.type_);
  out << (int)(s.key_.size());
  out << s.key_;
  out << (int)(s.value_.size());
  out << s.value_;
  return out;
}

istream& operator>>(istream& in, Log& s) {
  int res;
  char *a;
  
  in >> res;
  s.type_ = (WalType)res;

  in >> res;
  a = new char(res + 1);
  in.read(a, res);
  a[res] = '\0';
  s.key_ = std::string(a);
  delete a;

  in >> res;
  a = new char(res + 1);
  in.read(a, res);
  a[res] = '\0';
  s.value_ = std::string(a);
  delete a;
  return in;
}

class Wal {
 public:
  Wal();
  ~Wal();
  Status Init(const std::string& path);
  Status Start();
  Status Recover();
  Status LogToDisk(const Log& log);
  Status LogToDiskWithoutLock(const Log& log);
  Status AppendToMLog(const Log& log);
  Status SyncToDLog();
 private:
  std::vector<Log> mlog_;
  ofstream dlog_;
  std::string log_file_;
  std::mutex log_mu_;
};

}  // namespace wal
}  // namespace ctgfs
