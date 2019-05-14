// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <storage/memtable/memtable.h>
#include <storage/sstable/flusher.h>

namespace ctgfs {
namespace memtable {

using util::Status;
using namespace util;
using namespace sstable;

Memtable::Memtable(uint64_t max_size, std::string dir)
  : init_(false), is_flushing_(false),
    data_size_(0), max_size_(max_size),
    mem_map_(nullptr), i_mem_map_(nullptr),
    sst_mgr_(dir), dir_(dir) {}
    
Memtable::~Memtable() {}

Status Memtable::Init() {
  Status ret = Status::OK();
  init_ = true;
  mem_map_ = std::make_shared<std::set<Log>>();
  sst_mgr_.Init();
  return ret;
}

Status Memtable::Stop() {
  Status ret = Status::OK();
  ret = sst_mgr_.Stop();
  init_ = false;
  return ret;
}

Status Memtable::Add(const Log& log) {
  Status ret = Status::OK();
  std::lock_guard<std::mutex> guard(mu_);

  if (data_size_ > max_size_) {
    ret = MinorFreeze();
  } else if (ret.IsOK()) {
    mem_map_->insert(log);  
    data_size_ += log.Size();
    if (data_size_ > max_size_) {
      ret = MinorFreeze();
    }
  }

  return ret;
}

Status Memtable::MinorFreeze() {
  Status ret = Status::OK();
  i_mem_map_ = mem_map_;
  mem_map_ = std::make_shared<std::set<Log>>();
  data_size_ = 0;
  Log log = *(i_mem_map_->end());
  MemIterator mem_iter(i_mem_map_);
  ret = sst_mgr_.Flush(mem_iter, log);
  i_mem_map_ = nullptr;
  return ret;
}


bool MemIterator::HasNext() {
  bool bool_ret = false;
  if (iter_ == mt_->end()) {
    bool_ret = false;
  } else {
    bool_ret = true;
  }
  return bool_ret;
}

Status MemIterator::Next(Log &log) {
  Status ret = Status::OK();

  if (iter_ == mt_->end()) {
    ret = Status::Corruption("no next");
    printf("next for non next\n");
  } else {
    log = *iter_;
    iter_++;
  }

  return ret;
}

} // namespace memtable
} // namespace ctgfs
