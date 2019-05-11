// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h> 
#include <fcntl.h> 
#include <assert.h>
#include <limits>

namespace ctgfs {
namespace storage {

using util::Status;
using namespace util;
using namespace std;

std::string uint64_to_str(uint64_t i);
int str_to_int(const char *s);
uint64_t str_to_uint64(const char *s);
std::string int_to_str(int i);

struct Log {
  enum Op { PUT, DEL };
  Log(uint64_t l, const Op &o, const std::string &k, const std::string &v)
    : lsn(l), size(0), key(k), value(v), op(o) {
    size = 3 * sizeof(uint64_t) + key.size() + value.size() + sizeof(Op);
  }
  Log() :lsn(std::numeric_limits<uint64_t>::max()), size(0), key(""), value("") {}
  virtual ~Log() {};
  bool IsValid() { return lsn != std::numeric_limits<uint64_t>::max(); }
  void Reset() { lsn = std::numeric_limits<uint64_t>::max(); }
  Status Encode(std::string &bytes) const {
    Status ret = Status::OK();
    bytes.clear();
    bytes += uint64_to_str(lsn);
    bytes += int_to_str((int)(op));
    bytes += uint64_to_str((uint64_t)(key.size()));
    bytes += key;
    bytes += uint64_to_str((uint64_t)(value.size()));
    bytes += value;
    return ret;
  }

  Status Decode(const std::string &bytes) {
    Status ret = Status::OK();
    uint64_t size1 = 0;
    uint64_t size2 = 0;
    const char *bytep = bytes.c_str();
    lsn = str_to_uint64(bytep);
    op = (Op)str_to_uint64(bytep + sizeof(uint64_t));
    size1 = str_to_uint64(bytep + sizeof(uint64_t) + sizeof(int));
    key = bytes.substr(2 * sizeof(uint64_t) + sizeof(int), size1);
    size2 = str_to_uint64(bytep + 2 * sizeof(uint64_t) + sizeof(int) + size1);
    value = bytes.substr(3 * sizeof(uint64_t) + sizeof(int) + size1, size2);
    size = size1 + size2 + sizeof(op) + 3 * sizeof(uint64_t);
    return ret;
  }

  Status Write(int fd) {
    Status ret = Status::OK();
    uint64_t tmp_size = size + sizeof(uint64_t);

    if (write(fd, &tmp_size, sizeof(uint64_t)) < 0) {
      printf("write fail when write log");
      ret = Status::Corruption("write fail when write log");
    } else {
      std::string tmp_str;

      if (!(ret = Encode(tmp_str)).IsOK()) {
        printf("Encode fail when write log");
      } else if (write(fd, tmp_str.c_str(), size) < 0) {
        printf("write fail when write log");
        ret = Status::Corruption("write fail when write log");
      }
    }

    return ret;
  }

  Status Write(int fd, uint64_t &writed) {
    Status ret = Status::OK();
    writed = size + sizeof(uint64_t);

    if (write(fd, &writed, sizeof(uint64_t)) < 0) {
      printf("write fail when write log");
      ret = Status::Corruption("write fail when write log");
    } else {
      std::string tmp_str;

      if (!(ret = Encode(tmp_str)).IsOK()) {
        printf("Encode fail when write log");
      } else if (write(fd, tmp_str.c_str(), size) < 0) {
        printf("write fail when write log");
        ret = Status::Corruption("write fail when write log");
      }
    }

    return ret;
  }

  Status Read(int fd) {
    Status ret = Status::OK();
    uint64_t readed = 0;

    if (read(fd, &readed, sizeof(uint64_t)) < 0) {
      printf("read fail when read log");
      ret = Status::Corruption("read fail when read log");
    } else {
      char *buf = new char[readed - sizeof(uint64_t)];
      if (read(fd, buf, readed - sizeof(uint64_t)) < 0) {
        printf("read fail when read log");
        ret = Status::Corruption("read fail when read log");
      } else if (!(ret = Decode(std::string(buf, readed - sizeof(uint64_t)))).IsOK()) {
        printf("decode fail when read log");
      } else {
        assert(size == readed - sizeof(uint64_t));
      }
    }

    return ret;
  }

  Status Read(int fd, uint64_t &readed) {
    Status ret = Status::OK();

    if (read(fd, &readed, sizeof(uint64_t)) < 0) {
      printf("read fail when read log");
      ret = Status::Corruption("read fail when read log");
    } else {
      char *buf = new char[readed - sizeof(uint64_t)];
      if (read(fd, buf, readed - sizeof(uint64_t)) < 0) {
        printf("read fail when read log");
        ret = Status::Corruption("read fail when read log");
      } else if (!(ret = Decode(std::string(buf, readed - sizeof(uint64_t)))).IsOK()) {
        printf("decode fail when read log");
      } else {
        assert(size == readed - sizeof(uint64_t));
      }
    }

    return ret;
  }

  uint64_t Size() const {
    return size;
  }

  uint64_t lsn;
  uint64_t size;
  std::string key;
  std::string value;
  Op op;
};

bool operator<(const Log& left, const Log& right);

}  // namespace storage
}  // namespace ctgfs
