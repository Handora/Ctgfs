// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <storage/log.h>

namespace ctgfs {
namespace storage {

std::string uint64_to_str(uint64_t i) {
  return std::string((char*)(&i), sizeof(i));
}

int str_to_int(const char* s) { return *(int*)(s); }

uint64_t str_to_uint64(const char* s) { return *(uint64_t*)(s); }

std::string int_to_str(int i) { return std::string((char*)(&i), sizeof(i)); }

bool operator<(const Log& left, const Log& right) {
  bool ret = true;
  if (left.key < right.key) {
    ret = true;
  } else if (left.key > right.key) {
    ret = false;
  } else {
    ret = left.lsn > right.lsn;
  }

  return ret;
}

}  // namespace storage
}  // namespace ctgfs
