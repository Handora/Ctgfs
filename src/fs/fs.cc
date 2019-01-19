/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <cassert>
#include <fs/fs.h>
#include <util/json.h>

namespace ctgfs {
namespace fs {
using namespace util;

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
  size_t len = 0;
  assert((len = path.size()) != 0 && path[0] == '/');

  std::string name, tar;
  std::vector<std::string> folders;
  std::istringstream iss(path);
  while (getline(iss, name, '/')) {
    folders.emplace_back(name);   
  }
  tar = folders.back();
  folders.pop_back();

  int cur_type = -1;
  std::string cur_dir = "/", content;
  std::vector<std::string> children;
  kv_->Get("/", content); 
  decodeFile(content, children, cur_type);
  std::vector<std::string>::iterator i;
  for (i = folders.begin(); i != folders.end(); i++) {
    if(cur_type != FileType::kFolder) {
      return Status::InvalidArgument("The path:" + path + " is invalid."); 
    }

    const std::string& cur_folder = *i;
    auto iter = std::find(children.begin(), children.end(), cur_folder);
    if (iter != children.end()) {
      iter++; // folders { name, guid, name, guid, ... }
      assert(iter != children.end());
      content = "";
      kv_->Get(*iter, content);
      decodeFile(content, children, cur_type);
    } else {
      return Status::InvalidArgument("folder: " + cur_folder + " is invalid."); 
    }
  } 

  if (i == folders.end()) {
    if(cur_type == FileType::kFolder) {
      auto iter = std::find(children.begin(), children.end(), tar);
      if (iter != children.end()) {
        iter++;
        assert(iter != children.end());
        key = *iter;
      }
    } else {
      return Status::InvalidArgument("folder: " + tar + " is invalid."); 
    }
  } else {
    assert(false);
  }

  return Status::OK();
}

Status FileSystem::encodeFile(const std::string& path, std::string& result, int type) const {

}

Status FileSystem::decodeFile(const std::string& content, std::vector<std::string>& result, int& type) const {
  // assume that some characters are not allowed to appear in the file name, such as '|', '/'.
  // now, content is composed of '|', 'file_name/file_uuid' and '0' or '1'.

  type = content[0] - '0';
  if (type == FileType::kFile) {
    result.emplace_back(content.substr(2));
  } else if (type == FileType::kFolder) {
    std::istringstream iss(content.substr(2));
    std::string name_with_uuid;   // file_name/file_uuid
    while (getline(iss, name_with_uuid, '|')) {
      size_t cut = name_with_uuid.find_first_of("/");
      if (cut != std::string::npos) {
        result.emplace_back(name_with_uuid.substr(0, cut));     // push the name
        result.emplace_back(name_with_uuid.substr(cut + 1));    // push the guid value
      } else {
        return Status::InvalidArgument("failed to decode the content");
      }
    }
  } else {
    type = -1;
    result.clear();
    return Status::InvalidArgument("failed to decode the content");
  }

  return Status::OK();
}

} // namespace fs
} // namespace ctgfs
