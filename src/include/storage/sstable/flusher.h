// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <storage/sstable/sstable.h>
#include <storage/interface.h>

namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;
using namespace storage;

class SSTFlusher : public Flusher {
 public:
  SSTFlusher() : init_(false) {};
  virtual ~SSTFlusher() {};
  Status Init() override;
  Status Stop() override;
  Status Flush(const std::string &dir, const std::string &filename,
               Iterator &mem_iter, const Log &last_log, SStable &sst) override;
 private:
  bool init_;
};

}  // namespace sstable
}  // namespace ctgfs
