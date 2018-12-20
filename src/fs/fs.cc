/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#include <string>
#include <vector>
#include <memory>

#include <fs/fs.h>

namespace ctgfs {
namespace fs {

bool FileSystem::CreateDir(const std::string& path) {
  // todo 
  return true;
}

bool FileSystem::ReadDir(const std::string& path, std::vector<std::string>& children) const {
  // todo    
  return true;
}

bool FileSystem::RemoveDir(const std::string& path) {
  // todo 
  return true;
}

bool FileSystem::WriteFile(const std::string& path, const std::string& content) {
  // todo 
  return true;
}

bool FileSystem::ReadFile(const std::string& path, std::string& content) const {
  // todo 
  return true;
}

bool FileSystem::RemoveFile(const std::string& path) {
  // todo
  return true;
}

bool FileSystem::ChangeDir(const std::string& path, std::string& key) const {
  // todo  
  return true;
}

} // namespace fs
} // namespace ctgfs
