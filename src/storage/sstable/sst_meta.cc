// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <storage/sstable/sst_meta.h>
#include <stdio.h>

namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;

std::string uint64_to_str(uint64_t i) {
  return std::string((char*)(&i), sizeof(i));
}

uint64_t str_to_uint64(const char *s) {
  return *(uint64_t*)(s);
}

Status SSTMeta::Encode(std::string& meta) {
  Status ret = Status::OK();
  meta.clear();

  std::string data;
  if (!(ret = last_log_.Encode(data)).IsOK()) {
    printf("can't log last_log_\n");
  } else {
    meta += data;
    meta += uint64_to_str(data_offset_);
    meta += uint64_to_str(data_size_);

    if (meta.size() != Size()) {
      ret = Status::Corruption("encodconst e to a different dumber");
    }
  }

  return ret;
}

Status SSTMeta::Decode(const std::string &meta) {
  Status ret = Status::OK();

  if (!(ret = last_log_.Decode(meta)).IsOK()) {
    printf("last_log_ deccode error\n");
  } else {
    uint64_t size = last_log_.Size();
    data_offset_ = str_to_uint64(meta.c_str() + size);
    data_size_ = str_to_uint64(meta.c_str() + size + sizeof(uint64_t));
  }
  return ret;
}

Status SSTMeta::Write(int fd) {
  Status ret = Status::OK();
  uint64_t tmp_size = Size() + sizeof(uint64_t);

  if (write(fd, &tmp_size, sizeof(uint64_t)) < 0) {
    printf("write fail when write meta");
    ret = Status::Corruption("write fail when write meta");
  } else {
    std::string tmp_str;

    if (!(ret = Encode(tmp_str)).IsOK()) {
      printf("Encode fail when write meta");
    } else if (write(fd, tmp_str.c_str(), Size()) < 0) {
      printf("write fail when write meta");
      ret = Status::Corruption("write fail when write meta");
    }
  }

  return ret;
}

Status SSTMeta::Write(int fd, uint64_t &writed) {
  Status ret = Status::OK();
  writed = Size() + sizeof(uint64_t);

  if (write(fd, &writed, sizeof(uint64_t)) < 0) {
    printf("write fail when write meta");
    ret = Status::Corruption("write fail when write meta");
  } else {
    std::string tmp_str;

    if (!(ret = Encode(tmp_str)).IsOK()) {
      printf("Encode fail when write meta");
    } else if (write(fd, tmp_str.c_str(), Size()) < 0) {
      printf("write fail when write meta");
      ret = Status::Corruption("write fail when write meta");
    }
  }

  return ret;
}

Status SSTMeta::Read(int fd) {
  Status ret = Status::OK();
  uint64_t readed = 0;

  if (read(fd, &readed, sizeof(uint64_t)) < 0) {
    printf("read fail when read meta");
    ret = Status::Corruption("read fail when read meta");
  } else {
    char *buf = new char[readed - sizeof(uint64_t)];
    if (read(fd, buf, readed - sizeof(uint64_t)) < 0) {
      printf("read fail when read meta");
      ret = Status::Corruption("read fail when read meta");
    } else if (!(ret = Decode(std::string(buf, readed - sizeof(uint64_t)))).IsOK()) {
      printf("decode fail when read meta");
    } else {
      assert(Size() == readed - sizeof(uint64_t));
    }
  }

  return ret;
}

Status SSTMeta::Read(int fd, uint64_t &readed) {
  Status ret = Status::OK();

  if (read(fd, &readed, sizeof(uint64_t)) < 0) {
    printf("read fail when read meta");
    ret = Status::Corruption("read fail when read meta");
  } else {
    char *buf = new char[readed - sizeof(uint64_t)];
    if (read(fd, buf, readed - sizeof(uint64_t)) < 0) {
      printf("read fail when read log");
      ret = Status::Corruption("read fail when read meta");
    } else if (!(ret = Decode(std::string(buf, readed - sizeof(uint64_t)))).IsOK()) {
      printf("decode fail when read meta");
    } else {
      assert(Size() == readed - sizeof(uint64_t));
    }
  }

  return ret;
}


}  // namespace sstable
}  // namespace ctgfs

