/*
 * author: wfgu (peter.wfgu@gmail.com)
 * */

#pragma once

#include <string>
#include <vector>
#include <kv/kv.h>
#include <util/status.h>

namespace ctgfs {
namespace fs {

// 0: the file type
// 1: the folder type
enum FileType{
  kFile = 0,
  kFolder
};

using util::Status;

class IFileSystem {
 public:
  // Create a directory with its absolute path. 
  // Return virtual Status to indicate success or not.
  virtual Status CreateDir(const std::string& path) = 0;
  
  // Get the content of a folder with its absolute path. 
  // Return virtual Status to indicate success or not.
  virtual Status ReadDir(const std::string& path, std::vector<std::string>& children) const = 0;

  // Delete a directory with its absolute path.
  // Return virtual Status to indicate success or not.
  virtual Status RemoveDir(const std::string& path) = 0;
  
  // Write content into a file by its absolute path.
  // Return virtual Status to indicate success or not.
  virtual Status WriteFile(const std::string& path, const std::string& content) = 0;

  // Get the content of a file by its absolute path. 
  // Return virtual Status to indicate success or not.
  virtual Status ReadFile(const std::string& path, std::string& content) const = 0;

  // Delete a file with its absolute path. 
  // Return virtual Status to indicate success or not.
  virtual Status RemoveFile(const std::string& path) = 0;

 private:
  // Change the directory with an absolute path. If success, get the key of the folder.
  // Return virtual Status to indicate success or not.
  virtual Status parsePath(const std::string& path, std::string& key) const = 0;

  virtual Status encodeFile(const std::string& path, std::string& result, int type) const = 0;
  virtual Status decodeFile(const std::string& content, std::vector<std::string>& result, int& type) const = 0;
};

class FileSystem : public IFileSystem {
 public:
  Status CreateDir(const std::string& path) override;
  Status ReadDir(const std::string& path, std::vector<std::string>& children) const override;
  Status RemoveDir(const std::string& path) override;
  Status WriteFile(const std::string& path, const std::string& content) override;
  Status ReadFile(const std::string& path, std::string& content) const override;
  Status RemoveFile(const std::string& path) override;

 private:
  Status parsePath(const std::string& path, std::string& key) const override;
  Status encodeFile(const std::string& path, std::string& result, int type) const override;
  Status decodeFile(const std::string& content, std::vector<std::string>& result, int& type) const override;

 private:
  std::shared_ptr<kv::KV> kv_;
};

} // namespace fs
} // namespace ctgfs
