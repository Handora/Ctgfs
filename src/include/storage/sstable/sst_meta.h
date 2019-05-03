// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <string>
#include <mutex>
#include <map>
#include <storage/log.h>
#include <storage/interface.h>
#include <memory>

namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;
using namespace storage;;

// TODO: add bloom filter
class SSTMeta {
 public:
  SSTMeta(const Log &log, uint64_t offset, uint64_t size)
    : last_log_(log), data_offset_(offset), data_size_(size)
  {}
  SSTMeta()
    : data_offset_(0), data_size_(0) {}
  virtual ~SSTMeta() {};
  uint64_t Size() {
    return last_log_.Size() + 2 * sizeof(uint64_t);
  }
  Status Encode(std::string& meta);
  Status Decode(const std::string& meta);
 public:
  Log last_log_;
  uint64_t data_offset_;
  uint64_t data_size_;
};

}  // namespace sstable
}  // namespace ctgfs
