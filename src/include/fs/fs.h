/*
 * author: wfgu (peter.wfgu@gmail.com)
 * */

#pragma once

#include <string>
#include <vector>

namespace ctgfs {
namespace fs {

// 0: the file type
// 1: the folder type
enum FileType{
  kFile = 0,
  kFolder
};

class IFileSystem {
 public:
  // Create a directory with its absolute path. 
  // Return bool to indicate success or not.
  virtual bool CreateDir(const std::string& path)=0;
  
  // Get the content of a folder with its absolute path. 
  // Return bool to indicate success or not.
  virtual bool ReadDir(const std::string& path, std::vector<std::string>& children) const=0;

  // Delete a directory with its absolute path.
  // Return bool to indicate success or not.
  bool RemoveDir(const std::string& path);
  
  // Write content into a file by its absolute path.
  // Return bool to indicate success or not.
  bool WriteFile(const std::string& path, const std::string& content);

  // Get the content of a file by its absolute path. 
  // Return bool to indicate success or not.
  bool ReadFile(const std::string& path, std::string& content) const;

  // Delete a file with its absolute path. 
  // Return bool to indicate success or not.
  bool RemoveFile(const std::string& path);

 private:
  // This is an aid function.
  // Change the directory with an absolute path. If success, get the key of the folder.
  // Return bool to indicate success or not.
  bool ChangeDir(const std::string& path, std::string& key) const;
};

} // namespace fs
} // namespace ctgfs
