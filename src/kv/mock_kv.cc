// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <kv/mock_kv.h>
#include <string>
#include <algorithm>

namespace ctgfs {
namespace kv {

bool MockKV::Put(const std::string& key, const std::string& value) {
  int ret = true;
  kvs_[key] = value;
  return ret;
}

bool MockKV::Get(const std::string& key, std::string& value) {
  int ret = false;
  if (kvs_.count(key) > 0) {
    value = kvs_[key];
    ret = true;
  }
  return ret;
}

bool MockKV::Query(const std::string& key, std::map<std::string, std::string>& values) {
  int ret = true;
  for (auto it = kvs_.begin(); it != kvs_.end(); it++) {
    // for example key = a, *iter = a_<uint64_t binary number>
    if (key.size() + 1 < it->first.size()) {
      if (key == it->first.substr(0, key.size()) && (it->first)[key.size()] == '_') {
        values.insert(*it);        
      }
    }
  }
  return ret;
}
}  // namespace kv
}  // namespace ctgfs
