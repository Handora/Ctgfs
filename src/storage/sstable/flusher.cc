// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <storage/sstable/flusher.h>


namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;
using namespace storage;

Status SSTFlusher::Init() {
  Status ret = Status::OK();
  init_ = true;
  return ret;
}

Status SSTFlusher::Stop() {
  Status ret = Status::OK();
  init_ = false;
  return ret;
}

Status SSTFlusher::Flush(const std::string &dir, const std::string &filename,
               Iterator &mem_iter, const Log &last_log, SStable &sst) {
  Status ret = Status::OK();
  if (!(ret = sst.Init(dir, filename, last_log)).IsOK()) {
    printf("init sst error\n");
  } else {
    Log log;
    while (ret.IsOK() && mem_iter.HasNext()) {
      if (!(ret = mem_iter.Next(log)).IsOK()) {
        printf("get mem_iter next error\n");
      }
      sst.Append(log);
    }
    
    if (ret.IsOK() && !(ret = sst.Flush()).IsOK()) {
      printf("flush sst error\n");
    }
  }

  return ret;
}

}  // namespace sstable
}  // namespace ctgfs
