// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <util/status.h>
#include <storage/memtable/wal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <functional>

namespace ctgfs {
namespace memtable {

using util::Status;
using namespace util;
using namespace storage;

Wal::Wal() 
  : init_(false), dlog_(0), dir_(""),
    filename_(""), current_(""), func_() {}

Wal::~Wal() {}

Status Wal::Init(const std::string& dir, const std::function<void(const Log&)>& f) {
  Status ret = Status::OK();
  
  func_ = f;
  dir_ = dir;
  if (dir.size() == 0) {
    ret = Status::InvalidArgument("dir size is 0");
  } else if (dir[dir.size() - 1] == '/') {
    filename_ = "log.ctgfs";  
    current_ = "CURRENT";
  } else {
    filename_ = "/log.ctgfs";   
    current_ = "/CURRENT";
  }

  if(access((dir_ + filename_).c_str(), F_OK) != -1 &&
      access((dir_ + current_).c_str(), F_OK) != -1) {
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
  uint64_t w = 0;

  dlog_ = open((dir_ + filename_).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); 
  if (dlog_ < 0) {
    ret = Status::Corruption("Open fail\n");
    printf("open file fail\n");
  } else {
    int current_fd = open((dir_ + current_).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (current_fd < 0) {
      ret = Status::Corruption("Open fail\n");
      printf("open file fail\n");
    } else if (write(current_fd, &w, sizeof(int64_t)) < 0) {
      ret = Status::Corruption("write fail\n");
      printf("write file fail\n");
    } else {
      close(current_fd);
    }
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
  Log log;
  struct stat st;
  uint64_t size = 0;
  uint64_t offset = 0;

  int reader = open((dir_ + current_).c_str(), O_RDONLY);
  if (reader < 0) {
    ret = Status::Corruption("Open fail\n");
    printf("open file fail\n");
  } else if (read(reader, &offset, sizeof(uint64_t)) < 0) {
    ret = Status::Corruption("Read fail\n");
    printf("read current fail\n");
  } else {
    close(reader);
    reader = open((dir_ + filename_).c_str(), O_RDONLY);

    if (reader < 0) {
      ret = Status::Corruption("Open fail\n");
      printf("open file fail\n");
    } else {
      stat((dir_ + filename_).c_str(), &st);
      size = st.st_size;
      if (lseek(reader, offset, SEEK_SET) < 0) {
        ret = Status::Corruption("seek error");
        printf("seek error\n");
      }
    }
  }

  while (ret.IsOK() && offset < size) {
    uint64_t readed = 0;
    ret = log.Read(reader, readed);
    if (ret.IsOK()) {
      func_(log);
      offset += readed;
    }
  }

  if (ret.IsOK()) {
    close(reader);
    dlog_ = open((dir_ + filename_).c_str(), O_WRONLY, 0644); 
    if (dlog_ < 0) {
      ret = Status::Corruption("Open fail\n");
      printf("open file fail\n");
    } else if (lseek(dlog_, 0, SEEK_END) < 0) {
      ret = Status::Corruption("seek fail\n");
      printf("seek file fail\n");
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

Status Wal::LogCurrent(uint64_t offset) {
  Status ret = Status::OK();

  int fd = open((dir_ + current_).c_str(), O_WRONLY | O_CREAT | O_TRUNC);
  if (fd < 0) {
    ret = Status::Corruption("Open fail\n");
    printf("open file fail\n");
  } else if (write(fd, &offset, sizeof(uint64_t)) < 0) {
    ret = Status::Corruption("write fail\n");
    printf("write current fail\n");
  } else {
    close(fd);
  }
  return ret;
}

Status Wal::LogCurrent() {
  Status ret = Status::OK();
  uint64_t offset = GetDiskLogSize();

  int fd = open((dir_ + current_).c_str(), O_WRONLY | O_CREAT | O_TRUNC);
  if (fd < 0) {
    ret = Status::Corruption("Open fail\n");
    printf("open file fail\n");
  } else if (write(fd, &offset, sizeof(uint64_t)) < 0) {
    ret = Status::Corruption("write fail\n");
    printf("write current fail\n");
  } else {
    close(fd);
  }
  return ret;
}

uint64_t Wal::GetDiskLogSize() {
  struct stat st;
  stat((dir_ + filename_).c_str(), &st);
  return st.st_size;
}
}  // namespace wal
}  // namespace ctgfs
