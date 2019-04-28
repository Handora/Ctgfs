// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <util/status.h>
#include <wal/wal.h>

namespace ctgfs {
namespace wal {

using util::Status;
using namespace util;

Wal::Wal() 
  : log_file_("") {}

Wal::~Wal() {}

Status Wal::Init(const std::string& path) {
  Status ret = Status::Status::OK();
  
  if (path.size() == 0) {
    ret = Status::InvalidArgument("path size is 0");
  } else if (path[path.size() - 1] == '/') {
    log_file_ = path + "log.ctgfs";  
  } else {
    log_file_ = path + "/log.ctgfs";   
  }

  std::ofstream tmp;
  tmp.open(log_file_, ios::in);
  if (ret.IsOK() && tmp.good()) {
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
  dlog_.open(log_file_, ios::app | ios::trunc );
  return ret;
}

Status Wal::Recover() {
  Status ret = Status::OK();
  std::ifstream reader;
  mlog_.clear();
  Log log;
  reader.open(log_file_, ios::in);
  if (!reader.is_open()) {
    ret = Status::Corruption("open error");
  } else {
    while (reader >> log) {
      mlog_.push_back(log);
    }
    reader.close();
    dlog_.open(log_file_, ios::app);
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
  mlog_.clear();

  return ret;
}

}  // namespace wal
}  // namespace ctgfs
