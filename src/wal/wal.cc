// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <util/status.h>
#include <wal/wal.h>

namespace ctgfs {
namespace wal {

using util::Status;
using namespace util;

std::string int_to_str(int i) {
  return std::string((char*)(&i), sizeof(i));
}

int str_to_int(char *s) {
  return *(int*)(s);
}

ostream& operator<<(ostream& out, const Log& s) {
  if (out << int_to_str((int)(s.type_))) {
    if (out << int_to_str((int)(s.key_.size()))) {
      if (out << s.key_) {
        if (out << int_to_str((int)(s.value_.size()))) {
          out << s.value_;
        }
      }
    }
  }
  return out;
}

bool operator==(const Log &left, const Log &right) {
  return left.type_ == right.type_
      && left.key_ == right.key_
      && left.value_ == right.value_;
}

istream& operator>>(istream& in, Log& s) {
  int res;
  char *a;
  char tmp[10];
  tmp[sizeof(int)] = '\0';
  
  if (in.read(tmp, sizeof(int))) {
    s.type_ = (WalType)(str_to_int(tmp));

    if (in.read(tmp, sizeof(int))) {
      res = str_to_int(tmp);
      a = new char(res + 1);
      in.read(a, res);
      a[res] = '\0';
      s.key_ = std::string(a);
      delete a;

      if (in.read(tmp, sizeof(int))) {
        res = str_to_int(tmp);
        a = new char(res + 1);
        in.read(a, res);
        a[res] = '\0';
        s.value_ = std::string(a);
        delete a;
      }
    }
  }

  return in;
}

Wal::Wal() 
  : log_file_("") {}

Wal::~Wal() {}

Status Wal::Init(const std::string& path) {
  Status ret = Status::OK();
  
  if (path.size() == 0) {
    ret = Status::InvalidArgument("path size is 0");
  } else if (path[path.size() - 1] == '/') {
    log_file_ = path + "log.ctgfs";  
  } else {
    log_file_ = path + "/log.ctgfs";   
  }

  std::ifstream tmp(log_file_, ios::in);
  if (ret.IsOK() && !tmp.fail()) {
    tmp.close();
    ret = Recover();
  } else if (ret.IsOK()) {
    tmp.close();
    ret = Start();
  }

  return ret;
}

Status Wal::Start() {
  Status ret = Status::OK();
  mlog_.clear();
  std::ofstream tmp(log_file_.c_str(), ios::trunc);
  tmp.close();
  dlog_.open(log_file_.c_str(), ios::out | ios::trunc );
  return ret;
}

Status Wal::Stop() {
  Status ret = Status::OK();
  SyncToDLog();
  dlog_.close();
  return ret;
}

Status Wal::Recover() {
  Status ret = Status::OK();
  std::ifstream reader;
  mlog_.clear();
  Log log;
  reader.open(log_file_.c_str(), ios::in);
  if (!reader.is_open()) {
    ret = Status::Corruption("open error");
  } else {
    while (reader >> log) {
      mlog_.push_back(log);
    }
    reader.close();
    dlog_.open(log_file_.c_str(), ios::app);
    if (!dlog_.is_open()) {
      ret = Status::Corruption("open error");
    }
  }
  return ret;
}

Status Wal::LogToDisk(const Log& log) {
  Status ret = Status::OK();
  std::lock_guard<std::mutex> guard(log_mu_);
  if (!dlog_.good()) {
    ret = Status::Corruption("log_ is not good");
  } else if (!dlog_.is_open()) {
    ret = Status::Corruption("log_ is not open");
  } else {
    dlog_ << log;
  }

  return ret;
}

Status Wal::LogToDiskWithoutLock(const Log& log) {
  Status ret = Status::OK();
  if (!dlog_.good()) {
    ret = Status::Corruption("log_ is not good");
  } else if (!dlog_.is_open()) {
    ret = Status::Corruption("log_ is not open");
  } else {
    dlog_ << log;
  }

  return ret;
}

Status Wal::AppendToMLog(const Log& log) {
  Status ret = Status::OK();
  std::lock_guard<std::mutex> guard(log_mu_);

  mlog_.push_back(log);
  return ret;
}

Status Wal::SyncToDLog() {
  Status ret = Status::OK();
  std::lock_guard<std::mutex> guard(log_mu_);

  for (unsigned int i = 0; ret.IsOK() && i < mlog_.size(); i++) {
    ret = LogToDiskWithoutLock(mlog_[i]);
  }
  dlog_.flush();
  mlog_.clear();

  return ret;
}

}  // namespace wal
}  // namespace ctgfs
