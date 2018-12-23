// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <kv/kv.h>
#include <string>

namespace ctgfs {
namespace kv {

// RaftKv is a kv store implemented using raft consensus algorithm
// that including 2*N+1 nodes and can tolerant N nodes failed.
//
// Example:
class RaftKV : public KV {
 public:
  // Given the key reference and value reference which you want to
  // put into the kv store, it will return bool to indicate success
  // or not.
  //
  // The function is re-entrant, so you can use it directly within
  // several threads.
  bool Put(const std::string& key, const std::string& value) override;

  // Given the key reference which you want to search within the kv
  // store, it will return bool to indicate sucess or not. If success,
  // the value will be the corresponding value. Otherwise, don't use
  // it.
  //
  // The function is re-entrant, so you can use it directly within
  // several threads.
  bool Get(const std::string& key, std::string& value) override;

 private:
  // TODO(Handora): Fill in private function and varaiable
};

}  // namespace kv
}  // namespace ctgfs
