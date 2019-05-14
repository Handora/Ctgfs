// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <string>
#include <mutex>
#include <set>
#include <storage/sstable/sst_mgr.h>
#include <storage/log.h>
#include <storage/interface.h>
#include <memory>

namespace ctgfs {
namespace memtable {

using util::Status;
using namespace util;
using namespace storage;
using namespace sstable;

class Memtable {
 public:
  Memtable(uint64_t max_size, std::string dir);
  virtual ~Memtable();
  Status Init();
  Status Stop();
  Status Add(const Log& log);
  Status Get(const std::string &key, std::string &value);
 private:
  Status MinorFreeze();
  bool init_;
  bool is_flushing_;
  uint64_t data_size_;
  uint64_t max_size_;
  std::shared_ptr<std::set<Log>> mem_map_;
  std::shared_ptr<std::set<Log>> i_mem_map_;
  std::mutex mu_;
  SSTMgr sst_mgr_;
  std::string dir_;
};

class MemIterator : public Iterator {
 public:
  MemIterator(const std::shared_ptr<std::set<Log>> &mt)
  : mt_(mt) { iter_ = mt_->begin(); };
  virtual ~MemIterator() {};
  bool HasNext() override;
  Status Next(Log &log) override;
 private:
  std::shared_ptr<std::set<Log>> mt_;
  std::set<Log>::iterator iter_;
};

}  // namespace memtable
}  // namespace ctgfs
