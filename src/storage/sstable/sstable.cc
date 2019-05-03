// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <storage/sstable/sstable.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h> 
#include <fcntl.h> 
#include <util/status.h> 

namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;
using namespace storage;

Status SStable::Init(const std::string &dir, const std::string &filename, const Log &last_log) {
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
    printf("fail open file when recover\n");
  } else if (!(ret = meta_.Read(fd)).IsOK()) {
    close(fd);
    printf("fail read file when recover\n");
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

Status SStable::Append(const Log& log) {
  Status ret = Status::OK();

  std::string tmp;
  if (!(ret = log.Encode(tmp)).IsOK()) {
    printf("encode log error\n");
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
  // if (!(ret = meta.Encode(tmp)).IsOK()) {
  //   printf("encode meta error\n");
  // } else {
  // encode meta
  // buffer += tmp;
  // encode buf
  buffer += buf_;
  // }

  return ret;
}

Status SStable::Flush() {
  Status ret = Status::OK();
  int fd = 0;
  if ((fd = open((dir_ + "/" + filename_).c_str(),  O_CREAT | O_WRONLY, 0666)) < 0) {
    ret = Status::Corruption("fail open file when flush");
    printf("fail open file when flush\n");
  } else if (!(ret = meta_.Write(fd)).IsOK()) {
    close(fd);
    printf("fail write file when flush\n");
  // } else if (!(ret = Encode(buffer)).IsOK()) {
  //   close(fd);
  //   printf("fail encode when flush\n");
  } else if (write(fd, buf_.c_str(), buf_.size()) < 0) {
    close(fd);
    printf("fail write file when flush\n");
  } else {
    close(fd);
  }

  return ret;
}

Status SStable::CreateIterator(SSTIterator &iter) {
  Status ret = Status::OK();

  if (!(ret = iter.Init(dir_, filename_, meta_.data_offset_)).IsOK()) {
    printf("error init iterator for sstable\n");
  }

  return ret;
}

Status SSTIterator::Init(const std::string &dir, const std::string &filename, uint64_t offset) {
  Status ret = Status::OK();
  struct stat st;

  fd_ = open((dir + "/" + filename).c_str(), O_RDONLY);
  lseek(fd_, offset, SEEK_SET);
  if (fd_ < 0) {
    ret = Status::Corruption("can't open file");
    printf("can't open file\n");
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
      printf("error next the iterator");
    } else {
      pos_ += readed;
    }
  }

  return ret;
}


} // namespce sstable
} // namespace ctgfs

