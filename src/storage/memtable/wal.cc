// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <util/status.h>
#include <storage/memtable/wal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  

namespace ctgfs {
namespace memtable {

using util::Status;
using namespace util;
using namespace storage;

Wal::Wal() 
  : init_(false), dlog_(0), dir_(""), filename_("") {}

Wal::~Wal() {}

Status Wal::Init(const std::string& dir) {
  Status ret = Status::OK();
  
  dir_ = dir;
  if (dir.size() == 0) {
    ret = Status::InvalidArgument("dir size is 0");
  } else if (dir[dir.size() - 1] == '/') {
    filename_ = "log.ctgfs";  
  } else {
    filename_ = "/log.ctgfs";   
  }

  if(access((dir_ + filename_).c_str(), F_OK) != -1) {
    ret = Recover();
  } else {
    ret = Start();
  }

  if (ret.IsOK()) {
    init_ = true;
  }

  return ret;
}

Status Wal::Start() {
  Status ret = Status::OK();

  mlog_.clear();
  dlog_ = open((dir_ + filename_).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); 
  if (dlog_ < 0) {
    ret = Status::Corruption("Open fail\n");
    printf("open file fail\n");
  }

  return ret;
}

Status Wal::Stop() {
  Status ret = Status::OK();
  if (init_) {
    close(dlog_);
  }
  return ret;
}

Status Wal::Recover() {
  Status ret = Status::OK();
  mlog_.clear();
  Log log;
  struct stat st;
  uint64_t size = 0;
  uint64_t offset = 0;

  int reader = open((dir_ + filename_).c_str(), O_RDONLY);
  if (reader < 0) {
    ret = Status::Corruption("Open fail\n");
    printf("open file fail\n");
  } else {
    stat((dir_ + filename_).c_str(), &st);
    size = st.st_size;
  }

  while (ret.IsOK() && offset < size) {
    uint64_t readed = 0;
    ret = log.Read(reader, readed);
    if (ret.IsOK()) {
      mlog_.push_back(log);
      offset += readed;
    }
  }

  if (ret.IsOK()) {
    close(reader);
    dlog_ = open((dir_ + filename_).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); 
    if (dlog_ < 0) {
      ret = Status::Corruption("Open fail\n");
      printf("open file fail\n");
    }
  }
  return ret;
}

Status Wal::LogToDisk(Log& log) {
  Status ret = Status::OK();
  std::lock_guard<std::mutex> guard(log_mu_);

  if (!(ret = log.Write(dlog_)).IsOK()) {
    printf("log write error");
  }
  return ret;
}

Status Wal::LogToDiskWithoutLock(Log& log) {
  Status ret = Status::OK();

  if (!(ret = log.Write(dlog_)).IsOK()) {
    printf("log write error");
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
  fsync(dlog_);
  mlog_.clear();

  return ret;
}

}  // namespace wal
}  // namespace ctgfs
