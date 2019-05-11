// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <string>
#include <storage/log.h>
#include <storage/interface.h>

namespace ctgfs {
namespace storage {

using util::Status;
using namespace util;

class CtgStore : public Store {
 public:
  CtgStore();
  virtual ~CtgStore();
  Status Init(const std::string& path) override;
  Status Start() override;
  Status Stop() override;
  Status Put(const std::string &key, const std::string &value) override;
  Status Get(const std::string &key, std::string &value) override;
  Status Delete(const std::string &key) override;
  Iterator<Log> Query(const std::string &Prefix) override;
};

}  // namespace wal
}  // namespace ctgfs
