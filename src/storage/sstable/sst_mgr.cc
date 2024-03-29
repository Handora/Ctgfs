// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <dirent.h>
#include <storage/sstable/sst_mgr.h>
#include <util/util.h>

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
      if (dir->d_type == DT_REG && strcmp(dir->d_name, ".") != 0 &&
          strcmp(dir->d_name, "..") != 0) {
        if (std::string(dir->d_name).find(SST_PREFIX) == 0) {
          files.push_back(std::string(dir->d_name));
        }
      }
    }
    closedir(d);
  } else {
    ret = Status::Corruption("can't open dir");
    CTG_WARN("can't open dir");
  }

  return ret;
}

Status SSTMgr::Init() {
  Status ret = Status::OK();
  std::vector<std::string> files;
  flusher_ = new SSTFlusher();
  if (!(ret = getFilesFromDir_(files)).IsOK()) {
    CTG_WARN("get files from dir");
  } else if (!(ret = Recover(files)).IsOK()) {
    CTG_WARN("recover from dir");
  } else if (!(ret = flusher_->Init()).IsOK()) {
    CTG_WARN("init flusher error");
  } else {
    id_ = 0;
    init_ = true;
  }

  return ret;
}

Status SSTMgr::Recover(const std::vector<std::string> &files) {
  Status ret = Status::OK();
  for (auto it = files.begin(); ret.IsOK() && it != files.end(); ++it) {
    std::string file = *it;
    SStable sst;
    if (!(ret = sst.Recover(dir_, file)).IsOK()) {
      CTG_WARN("fail to recover a sst");
    } else {
      ssts_.push_back(sst);
    }
  }

  return ret;
}

Status SSTMgr::Flush(Iterator &iter, const Log &last_log) {
  Status ret = Status::OK();
  SStable sst;

  if (!(ret = flusher_->Flush(dir_, nextFileName(), iter, last_log, sst))
           .IsOK()) {
    CTG_WARN("flusher flush error");
  } else {
    std::lock_guard<std::mutex> guard(ssts_mu_);
    ssts_.push_back(sst);
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
  delete flusher_;
  init_ = false;
  return ret;
}

}  // namespace sstable
}  // namespace ctgfs
