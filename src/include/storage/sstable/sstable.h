// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <string>
#include <mutex>
#include <map>
#include <storage/log.h>
#include <storage/interface.h>
#include <memory>
#include <storage/sstable/sst_meta.h>

namespace ctgfs {
namespace sstable {

using util::Status;
using namespace util;
using namespace storage;

class SSTIterator;

// Seperate SStable into SStable reader and SStable writer
class SStable {
 public:
  SStable() : init_(false) {};
  virtual ~SStable() {};
  Status Init(const std::string &dir, const std::string &filename, 
          const Log& last_log);
  Status Recover(const std::string &dir, const std::string &filename); 
  Status Stop();
  Status Append(const Log& log);
  Status Encode(std::string &buffer);
  Status CreateIterator(SSTIterator &iter);
  Status Flush();
 private:
  bool init_;
  std::string filename_;
  std::string dir_;
  SSTMeta meta_;
  std::string buf_;
};

class SSTIterator : public Iterator {
 public:
  SSTIterator() :
    init_(false), fd_(-1), pos_(0), size_(0) {};
  virtual ~SSTIterator() {};
  Status Init(const std::string &dir, const std::string &filename, uint64_t offset);
  Status Stop();
  bool HasNext() override;
  Status Next(Log &log) override;
 private:
  bool init_;
  int fd_;
  uint64_t pos_;
  uint64_t size_;
};

}  // namespace memtable
}  // namespace ctgfs
