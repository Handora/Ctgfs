// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <storage/memtable/memtable.h>
#include <storage/sstable/flusher.h>
#include <util/util.h>

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
  Log log = *(i_mem_map_->rbegin());
  MemIterator mem_iter;
  if (!(ret = mem_iter.Init(i_mem_map_)).IsOK()) {
    CTG_WARN("init memiter error");
  } else if (!(ret = sst_mgr_.Flush(mem_iter, log)).IsOK()) {
    CTG_WARN("flush sst_mgr error");
  } else {
    i_mem_map_ = nullptr;
  }
  return ret;
}

Status Memtable::CreateIterator(MemIterator &iter) {
  Status ret = Status::OK();
  std::lock_guard<std::mutex> guard(mu_);

  if (!(ret = iter.Init(mem_map_)).IsOK()) {
    CTG_WARN("error init iterator for memtable");
  }

  return ret;
}

Status MemIterator::Init(const std::shared_ptr<std::set<Log>> &mt) {
  Status ret = Status::OK();
  mt_ = mt;
  iter_ = mt_->begin(); 
  return ret;
};

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
    CTG_WARN("next for non next");
  } else {
    log = *iter_;
    iter_++;
  }

  return ret;
}

} // namespace memtable
} // namespace ctgfs
