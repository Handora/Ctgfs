/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <fs/fs.h>
#include <util/json.h>

namespace ctgfs {
namespace fs {
using util::Status;

Status FileSystem::CreateDir(const std::string& path) {
  // todo 
}

Status FileSystem::ReadDir(const std::string& path, std::vector<std::string>& children) const {
  // todo    
}

Status FileSystem::RemoveDir(const std::string& path) {
  // todo 
}

Status FileSystem::WriteFile(const std::string& path, const std::string& content) {
  // todo 
}

Status FileSystem::ReadFile(const std::string& path, std::string& content) const {
  // todo 
}

Status FileSystem::RemoveFile(const std::string& path) {
  // todo
}

Status FileSystem::parsePath(const std::string& path, std::string& key) const {
  /*
  size_t len = 0;
  assert((len = path.size()) != 0 && path[0] == '/');

  std::string cur_dir = "/", content;
  kv_->Get("/", content); 
  */
}

} // namespace fs
} // namespace ctgfs
