// Authors: Chen Qian(qcdsr970209@gmail.com) 

#pragma once
#include <kv/kv.h>
#include <string>
#include <map>

namespace ctgfs {
namespace kv {

class MockKV : public KV {
 public:
  MockKV() {}
  ~MockKV() {}
  bool Put(const std::string& key, const std::string& value) override;

  bool Get(const std::string& key, std::string& value) override;

  bool Query(const std::string& key, std::map<std::string, std::string>& values) override;
 private:
  std::map<std::string, std::string> kvs_;
};

}  // namespace kv
}  // namespace ctgfs
