#pragma once

#include <string>
#include "lockmanager/lock_manager.h"

namespace ctgfs {
namespace file_system {

class FileSystem {
 private:
  lock_manager::LockManager lock_manager_;
 public:
  bool CreateDirectory(const std::string& path, const std::string& value);
  bool RemoveDirectory(const std::string& path);

  bool ReadFile(const std::string& path, std::string& content) const;
};

} // namespace ctgfs
} // namespace file_system

