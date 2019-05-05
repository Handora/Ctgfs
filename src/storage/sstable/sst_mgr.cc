// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <storage/sstable/sst_mgr.h>
#include <dirent.h> 
#include <stdio.h> 

namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;
using namespace storage;

Status SSTMgr::getFilesFromDir_(std::vector<std::string> &files) {
  DIR *d;
  struct dirent *dir;
  Status ret = Status::OK();
  d = opendir(dir_.c_str());
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      printf("%s\n", dir->d_name);
      if (dir -> d_type == DT_REG && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
        if (std::string(dir->d_name).find(SST_PREFIX) == 0) {
          files.push_back(std::string(dir->d_name));
        }
      }
    }
    closedir(d);
  } else {
    ret = Status::Corruption("can't open dir");
    printf("can't open dir\n");
  }

  return ret;
}

Status SSTMgr::Init() {
  Status ret = Status::OK();
  std::vector<std::string> files;
  if (!(ret = getFilesFromDir_(files)).IsOK()) {
    printf("get files from dir\n");
  } else if (!(ret = Recover(files)).IsOK()) {
    printf("recover from dir\n");
  } else if (!(ret = flusher_->Init()).IsOK()) {
    printf("init flusher error\n");
  } else {
    id_ = 0;
    init_ = true;
  }

  return ret;
}

Status SSTMgr::Recover(const std::vector<std::string> &files) {
  Status ret = Status::OK();
  for (auto it = files.begin();
       ret.IsOK() && it != files.end(); ++it) {
    std::string file = *it;
    SStable sst;
    if (!(ret = sst.Recover(dir_, file)).IsOK()) {
      printf("fail to recover a sst\n");
    } else {
      ssts_.push_back(sst);
    }
  }

  return ret;
}

Status SSTMgr::Flush(Iterator<Log> &iter, const Log& last_log) {
  Status ret = Status::OK();
  SStable sst;

  if (!(ret = flusher_->Flush(dir_, nextFileName(), iter, last_log, sst)).IsOK()) {
    printf("flusher flush error\n");
  }

  return ret;
}

std::string SSTMgr::nextFileName() {
  char buffer[100];

  {
    std::lock_guard<std::mutex> guard(id_mu_);
    id_++;
  }
  sprintf(buffer, ".0x%lx", id_);
  return SST_PREFIX + std::string(buffer);
}

Status SSTMgr::Stop() {
  Status ret = Status::OK();
  flusher_->Stop();
  init_ = false;
  return ret;
}

} // namespace sstable
} // namespace ctgfs
