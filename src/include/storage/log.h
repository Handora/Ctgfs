// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <vector>
#include <iostream>

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
  Log() {}
  virtual ~Log() {};
  Status Encode(std::string &bytes) {
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
    size = 3 * sizeof(uint64_t) + key.size() + value.size() + sizeof(Op);
    return ret;
  }

  uint64_t Size() const {
    return 3 * sizeof(uint64_t) + key.size() + value.size() + sizeof(Op);
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
