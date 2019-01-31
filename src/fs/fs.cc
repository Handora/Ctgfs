/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <cassert>
#include <fs/fs.h>
#include <util/guid.h>

namespace ctgfs {
namespace fs {
using namespace util;

// for print debug message
const std::string red("\033[0;31m"); 
const std::string reset("\033[0m"); 

void debug(const std::string& msg) {
  std::cerr << red << msg << reset << std::endl;
}

FileSystem::FileSystem() noexcept {
  kv_ = std::make_shared< std::map<std::string, std::string> >( );
  (*kv_)[ROOT_KEY] = "1|"; // create root directory.
}

Status FileSystem::CreateDir(const std::string& path) {
  #ifdef dev
  debug("- command: mkdir " + path);
  #endif

  std::size_t found = path.find_last_of("/");
  if (found == std::string::npos) {
    return Status::InvalidArgument(path + " is invalid.");
  }

  std::string parent_dir_path = path.substr(0, found);
  if (parent_dir_path.empty() && found == 0) {
    /* when deal with "/a", 
     * parent_dir_path: " ", for the found = 0, so we need to fix it to "/"
     * target_dir_name: "a"
    * */
    parent_dir_path = "/";
  }
  std::string target_dir_name = path.substr(found + 1);
  if (target_dir_name.empty()) {
    // path is "/"
    return Status::InvalidArgument("/, file exists.");
  }

  #ifdef dev
  debug("- parent dir: " + parent_dir_path);
  debug("- target dir: " + target_dir_name);
  #endif

  // maybe, it's better to write some other functions like hasFile(find file, find folder) to make things easier.

  std::string parent_dir_key;
  Status status_get_parent_key = parsePath(parent_dir_path, parent_dir_key);
  if (!status_get_parent_key.IsOK()) {
    return status_get_parent_key;
  }

  #ifdef dev
  debug("- get key: " + parent_dir_key);
  #endif

  // get files under the directory.
  int type = -1;
  std::string parent_dir_raw_content, unused_content;
  std::vector<std::string> child_file_guid, child_file_name;
  Status status_get_dir = decodeFileFromKey(parent_dir_key, parent_dir_raw_content, unused_content, child_file_name, child_file_guid, type);
  if (!status_get_dir.IsOK()) {
    return status_get_dir;
  }
  if (type != 1) {
    return Status::InvalidArgument(parent_dir_path + "is not a directory.");
  }

  #ifdef dev
  // debug("- get subdirectory: ok");
  // std::size_t size_n = child_file_guid.size();
  // for (std::size_t i = 0; i < size_n; ++i) {
  //   debug("    - " + child_file_name[i] + " <-> " + child_file_guid[i]);
  // }
  #endif

  // check if there is a file/folder has the same name of the folder to be created.
  auto namehit= std::find(child_file_name.begin(), child_file_name.end(), target_dir_name);

  if (namehit != child_file_name.end()) {
    return Status::InvalidArgument(target_dir_name + "exists.");
  }


  // 1. generate an unique key.
  Guid target_guid = NewGuid();
  std::string target_dir_key = target_guid.str();

  // 2. append the new folder in the parent folder content
  // and update the parent directory in KV.
  parent_dir_raw_content += std::string(target_dir_name + "/" +  target_dir_key + "|");
  (*kv_)[parent_dir_key] = parent_dir_raw_content; // for test
  // kv_->Put(parent_dir_key, parent_dir_raw_content);


  // 3. put this new folder in KV.
  std::string target_dir_content = "1|"; // now it is empty.
  (*kv_)[target_dir_key] = target_dir_content; // for test
  // kv_->Put(target_dir_key, target_dir_content);
  

  #ifdef dev
  // for (const auto & kv : *kv_) {
  //   debug("\t" + kv.first + " <-> " + kv.second);
  // }
  #endif

  return Status::OK();
}

Status FileSystem::ReadDir(const std::string& path, std::vector<std::string>& children) const {
  #ifdef dev
  debug("- command: ls " + path);
  #endif

  std::size_t found = path.find_last_of("/");
  if (found == std::string::npos) {
    return Status::InvalidArgument(path + " is invalid.");
  }

  children.clear();

  // std::string parent_dir = path.substr(0, found);
  std::string target_dir = path.substr(found + 1);
  std::vector<std::string> child_file_guid;
  // unused in this section
  std::string raw_content, unused_content;
  int type = -1;

  // check whether the target directory is root.
  if (target_dir.empty()) {
    // path is "/"
    Status s = decodeFileFromKey(ROOT_KEY, raw_content, unused_content, children, child_file_guid, type);
    assert(type == 1 && s.IsOK());
    #ifdef dev
    // std::size_t k = children.size();
    // for (std::size_t i = 0; i < k; ++i) {
    //   debug("    - " + children[i] + " <-> " + child_file_guid[i]);
    // }
    #endif
    return s;
  }

  // 1. get the key of the target directory.
  std::string target_key;
  Status s = parsePath(path, target_key);
  if (!s.IsOK()) {
    return s;
  }

  // 2. get value from kv by key and deserialize the value to content.
  Status sret = decodeFileFromKey(target_key, raw_content, unused_content, children, child_file_guid, type);
  if (!sret.IsOK()) {
    return sret; 
  }
  if (type != 1) {
    return Status::InvalidArgument(target_dir + "is not a directory.");
  }

  #ifdef dev
  // std::size_t k = children.size();
  // for (std::size_t i = 0; i < k; ++i) {
  //   debug("    - " + children[i] + " <-> " + child_file_guid[i]);
  // }
  #endif

  return Status::OK();
}

Status FileSystem::RemoveDir(const std::string& path) {
  // todo 
  return Status::OK();
}

Status FileSystem::WriteFile(const std::string& path, const std::string& content) {
  // todo 
  return Status::OK();
}

Status FileSystem::ReadFile(const std::string& path, std::string& content) const {
  // todo 
  return Status::OK();
}

Status FileSystem::RemoveFile(const std::string& path) {
  // todo
  return Status::OK();
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
    if (!name.empty()) {
      folders.emplace_back(name);   
    }
  }
  if (folders.size() == 0) {
    // path is '/' or '//' ...
    key = ROOT_KEY;
    return Status::OK();;
  }
  
  int type = -1;
  std::string cur_dir_key = ROOT_KEY, cur_dir_name = "/"; // The path to be parsed must start from '/'
  std::string raw_content, unused_content;
  std::vector<std::string> child_file_name, child_file_guid;
  std::vector<std::string>::iterator it;
  for (it = folders.begin(); it != folders.end(); ++it) {
    const std::string& cur_target = *it;
    Status s = decodeFileFromKey(cur_dir_key, raw_content, unused_content, child_file_name, child_file_guid, type);
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

  return Status::OK();
}

Status FileSystem::encodeFile(const std::string& path, std::string& result, int type) const {
  // todo
  return Status::OK();
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
    std::string& raw_content, 
    std::string& file_content, 
    std::vector<std::string>& file_name, std::vector<std::string>& file_guid, 
    int& type) const 
{
  // maybe, need to validate the key here.
  
  if (kv_->count(key)) {
    raw_content = (*kv_)[key]; // for test
  }
  // kv_->Get(key, raw_content);

  // check the raw content got from kv::Get here.

  return decodeFile(raw_content, file_content, file_name, file_guid, type);
}

} // namespace fs
} // namespace ctgfs
