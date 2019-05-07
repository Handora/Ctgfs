// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <string>
#include <vector>
#include <storage/log.h>
#include <storage/sstable/sstable.h>

namespace ctgfs {
namespace storage {

using util::Status;
using sstable::SStable;
using namespace util;

class MultiIter final : public Iterator {
 public:
  // TODO: use Init 
  MultiIter(std::vector<Iterator*> &ssts);
  ~MultiIter() {}
  bool HasNext() override;
  Status Next(Log &log) override;
 private: 
  std::vector<Iterator*> iters_;
  std::vector<Log> logs_;
};
} // namespace storage
} // namespace ctgfs

