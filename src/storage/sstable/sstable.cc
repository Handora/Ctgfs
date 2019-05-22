// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <fcntl.h>
#include <stdio.h>
#include <storage/sstable/sstable.h>
#include <sys/stat.h>
#include <unistd.h>
#include <util/status.h>
#include <util/util.h>

namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;
using namespace storage;

Status SStable::Init(const std::string &dir, const std::string &filename,
                     const Log &last_log) {
  Status ret = Status::OK();
  filename_ = filename;
  dir_ = dir;
  meta_.last_log_ = last_log;
  meta_.data_offset_ = meta_.Size() + sizeof(uint64_t);
  buf_.clear();
  init_ = true;

  return ret;
}

Status SStable::Recover(const std::string &dir, const std::string &filename) {
  Status ret = Status::OK();
  filename_ = filename;
  dir_ = dir;
  buf_.clear();
  int fd = 0;
  if ((fd = open((dir + "/" + filename).c_str(), O_RDONLY)) < 0) {
    ret = Status::Corruption("fail open file when recover");
    CTG_WARN("fail open file when recover");
  } else if (!(ret = meta_.Read(fd)).IsOK()) {
    close(fd);
    CTG_WARN("fail read file when recover");
  } else {
    close(fd);
    init_ = true;
  }

  return ret;
}

Status SStable::Stop() {
  Status ret = Status::OK();
  init_ = false;

  return ret;
}

Status SStable::Append(const Log &log) {
  Status ret = Status::OK();

  std::string tmp;
  if (!(ret = log.Encode(tmp)).IsOK()) {
    CTG_WARN("encode log error");
  } else {
    uint64_t num = tmp.size() + sizeof(uint64_t);
    buf_ += std::string((char *)(&num), sizeof(uint64_t));
    buf_ += tmp;
    meta_.data_size_ += tmp.size();
  }

  return ret;
}

Status SStable::Encode(std::string &buffer) {
  Status ret = Status::OK();

  buffer.clear();
  buffer += buf_;

  return ret;
}

Status SStable::Flush() {
  Status ret = Status::OK();
  int fd = 0;
  if ((fd = open((dir_ + "/" + filename_).c_str(), O_CREAT | O_WRONLY, 0666)) <
      0) {
    ret = Status::Corruption("fail open file when flush");
    CTG_WARN("fail open file when flush");
  } else if (!(ret = meta_.Write(fd)).IsOK()) {
    close(fd);
    CTG_WARN("fail write file when flush");
  } else if (write(fd, buf_.c_str(), buf_.size()) < 0) {
    close(fd);
    CTG_WARN("fail write file when flush");
  } else {
    close(fd);
  }

  return ret;
}

Status SStable::CreateIterator(SSTIterator &iter) {
  Status ret = Status::OK();

  if (!(ret = iter.Init(dir_, filename_, meta_.data_offset_)).IsOK()) {
    CTG_WARN("error init iterator for sstable");
  }

  return ret;
}

Status SSTIterator::Init(const std::string &dir, const std::string &filename,
                         uint64_t offset) {
  Status ret = Status::OK();
  struct stat st;

  fd_ = open((dir + "/" + filename).c_str(), O_RDONLY);
  lseek(fd_, offset, SEEK_SET);
  if (fd_ < 0) {
    ret = Status::Corruption("can't open file");
    CTG_WARN("can't open file");
  } else {
    stat((dir + "/" + filename).c_str(), &st);
    size_ = st.st_size;
    pos_ = offset;
    init_ = true;
  }

  return ret;
}

Status SSTIterator::Stop() {
  Status ret = Status::OK();

  // TODO: solve error code
  close(fd_);
  init_ = false;
  return ret;
}

bool SSTIterator::HasNext() {
  bool ret = false;

  if (pos_ >= size_) {
    ret = false;
  } else {
    ret = true;
  }

  return ret;
}

Status SSTIterator::Next(Log &log) {
  Status ret = Status::OK();

  if (!HasNext()) {
    return Status::Corruption("no next");
  } else {
    uint64_t readed = 0;
    if (!(ret = log.Read(fd_, readed)).IsOK()) {
      CTG_WARN("error next the iterator");
    } else {
      pos_ += readed;
    }
  }

  return ret;
}

}  // namespce sstable
}  // namespace ctgfs
