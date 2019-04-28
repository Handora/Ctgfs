// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <map>
#include <string>

namespace ctgfs {
namespace kv {

// The kv store interface that you can use it to store or get key and
// value.
class KV {
 public:
  // Given the key reference and value reference which you want to
  // put into the kv store, it will return bool to indicate success
  // or not.
  virtual bool Put(const std::string& key, const std::string& value) = 0;

  // Given the key reference which you want to search within the kv
  // store, it will return bool to indicate sucess or not. If success,
  // the value will be the corresponding value. Otherwise, don't use
  // it.
  virtual bool Get(const std::string& key, std::string& value) = 0;

  // Given the predicate and get the value lists, use this for mvcc_get
  virtual bool Query(const std::string& key,
                     std::map<std::string, std::string>& values) = 0;
};

}  // namespace kv
}  // namespace ctgfs
