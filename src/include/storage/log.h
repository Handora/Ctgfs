// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <vector>

namespace ctgfs {
namespace storage {

using util::Status;
using namespace util;
using namespace std;

std::string uint64_to_str(uint64_t i) {
  return std::string((char*)(&i), sizeof(i));
}

int str_to_int(char *s) {
  return *(int*)(s);
}

int str_to_uint64(char *s) {
  return *(uint64_t*)(s);
}

std::string int_to_str(int i) {
  return std::string((char*)(&i), sizeof(i));
}

struct Log {
  enum Op { PUT, DEL };
  Log(const uint64_t l, const Op &o, const std::string &k, const std::string &v)
    : key(k), value(v), op(o), lsn(l) {
    size = sizeof(uint64_t) + key.size() + value.size() + sizeof(Op);
  }
  Log() {}
  virtual ~Log() {};
  Status Encode(std::string &bytes) {
    ret = OK();
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
    ret = OK();
    uint64_t size1 = 0;
    uint64_t size2 = 0;
    char *bytep = bytes.c_str();
    lsn = str_to_uint64(bytep);
    op = (Op)str_to_uint64(bytep + sizeof(uint64_t));
    size1 = str_to_uint64(bytep + sizeof(uint64_t) + sizeof(int));
    key = bytes.substr(2 * sizeof(uint64_t) + sizeof(int), size1);
    size2 = str_to_uint64(bytep + 2 * sizeof(uint64_t) + sizeof(int) + size1);
    value = bytes.sub_str(3 * sizeof(uint64_t) + sizeof(int) + size1, size2);
    return ret;
  }

  uint64_t lsn;
  std::string key;
  std::string value;
  Op op;
};

bool Operator <(const Log& left, const Log& right) {
  bool ret = true;
  if (left.key < right.key) {
    ret = true;
  } else if (left.key > right.key) {
    ret = false;
  } else {
    return left.lsn < right.lsn;
  }
}

}  // namespace wal
}  // namespace ctgfs
