/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#include <string>
#include <vector>

#include "fs/fs.h"

namespace ctgfs {
namespace fs {

bool FileSystem::CreateDir(const std::string& path) {
  // todo 
}

bool FileSystem::ReadDir(const std::string& path, std::vector<std::string>& children) const {
  // todo    
}

bool FileSystem::RemoveDir(const std::string& path) {
  // todo 
}

bool FileSystem::WriteFile(const std::string& path, const std::string& content) {
  // todo 
}

bool FileSystem::ReadFile(const std::string& path, std::string& content) const {
  // todo 
}

bool FileSystem::RemoveFile(const std::string& path) {
  // todo
}


} // namespace fs
} // namespace ctgfs
