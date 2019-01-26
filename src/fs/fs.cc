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

namespace ctgfs {
namespace fs {
using namespace util;

Status FileSystem::CreateDir(const std::string& path) {
  // todo 
}

Status FileSystem::ReadDir(const std::string& path, std::vector<std::string>& children) const {
  std::size_t found = path.find_last_of("/");
  if (found == std::string::npos) {
    return Status::InvalidArgument(path + " is invalid.");
  }

  std::string parent_dir = path.substr(0, found);
  std::string target_dir = path.substr(found + 1);
  std::string target_key, content, unused_content;
  std::vector<std::string> child_file_guid;
  int type = -1;
  if (target_dir.empty()) {
    // path is "/"
    Status s = decodeFileFromKey(ROOT_KEY, unused_content, children, child_file_guid, type);
    assert(type == 1 && s.IsOK());
    return s;
  }
  Status s = parsePath(path, target_key);
  if (!s.IsOK()) {
    return s;
  }
  Status sret = decodeFileFromKey(target_key, unused_content, children, child_file_guid, type);
  if (!sret.IsOK()) {
    return sret; 
  }
  if (type != 1) {
    return Status::InvalidArgument(target_dir + "is not a directory.");
  }
  return Status::OK();
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
  std::size_t len = path.size();
  // assert((len = path.size()) != 0 && path[0] == '/');
  if (len <= 0 || path[0] != '/') {
    return Status::InvalidArgument(path + " is invalid.");
  }

  std::string name;
  std::vector<std::string> folders;
  std::istringstream iss(path);
  while (getline(iss, name, '/')) {
    folders.emplace_back(name);   
  }
  if (folders.size() == 0) {
    // path is '/' or '//' ...
    key = ROOT_KEY;
    return Status::OK();;
  }
  std::string tar = folders.back();
  folders.pop_back();
  
  int type = -1;
  std::string cur_dir_key = ROOT_KEY, cur_dir_name = "/"; // The path to be parsed must start from '/'
  std::string raw_content, unused_content;
  std::vector<std::string> child_file_name, child_file_guid;
  std::vector<std::string>::iterator it;
  for (it = folders.begin(); it != folders.end(); ++it) {
    const std::string& cur_target = *it;
    Status s = decodeFileFromKey(cur_dir_key, unused_content, child_file_name, child_file_guid, type);
    if (!s.IsOK()) {
      return s;
    }
    if (type != 1) {
      return Status::InvalidArgument(cur_dir_name + " is not a folder.");
    }

    auto found = std::find(child_file_name.begin(), child_file_name.end(), cur_target);
    // do not find the target subdirectory
    if (found == child_file_name.end()) {
      return Status::InvalidArgument("cannot find " + cur_target + " in " + cur_dir_name);
    }
    // Find the target subdirectory.
    // there is a bug: the same name file and folder. 
    // So this file system do not support file and folder have the same name under one directory.
    auto target_index = std::distance(child_file_name.begin(), found);
    // get the key of target directory
    cur_dir_key = child_file_guid[target_index]; 
    cur_dir_name = cur_target;
  }
  if (it != folders.end()) {
    return Status::InvalidArgument("debug: error when parsing the path:." + path); 
  }

  // succeeded in parsing the path.
  key = cur_dir_key;
}

Status FileSystem::encodeFile(const std::string& path, std::string& result, int type) const {

}

Status FileSystem::decodeFile(
    const std::string& raw_content, 
    std::string& file_content, 
    std::vector<std::string>& file_name, std::vector<std::string>& file_guid, 
    int& type) const 
{
  // assume that some characters are not allowed to appear in the file name, such as '|', '/'.
  // now, the raw content is composed of '|', 'file_name/file_uuid' and '0' or '1'.

  type = raw_content[0] - '0';
  if (type == FileType::kFile) {
    file_content = raw_content.substr(2);
  } else if (type == FileType::kFolder) {
    std::istringstream iss(raw_content.substr(2));
    std::string name_with_uuid;   // file_name/file_uuid
    while (getline(iss, name_with_uuid, '|')) {
      std::size_t cut = name_with_uuid.find_first_of("/");
      if (cut != std::string::npos) {
        file_name.emplace_back(name_with_uuid.substr(0, cut));     // push the name
        file_guid.emplace_back(name_with_uuid.substr(cut + 1));    // push the guid value
      } else {
        return Status::InvalidArgument("debug: failed to decode the content");
      }
    }
  } else {
    type = -1;
    file_content = "";
    file_name.clear();
    file_guid.clear();
    return Status::InvalidArgument("debug: failed to decode the content");
  }

  return Status::OK();
}

Status FileSystem::decodeFileFromKey(
    const std::string& key,
    std::string& file_content, 
    std::vector<std::string>& file_name, std::vector<std::string>& file_guid, 
    int& type) const 
{
  std::string raw_content;
  // maybe, need to validate the key here.
  kv_->Get(key, raw_content);

  // check the result of the kv::Get

  return decodeFile(raw_content, file_content, file_name, file_guid, type);
}

} // namespace fs
} // namespace ctgfs
