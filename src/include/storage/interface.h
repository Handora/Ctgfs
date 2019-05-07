// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <string>
#include <storage/log.h>

namespace ctgfs {
namespace sstable {

class SStable;

} //namespace sstable

namespace storage {

using util::Status;
using sstable::SStable;
using namespace util;
using namespace std;

class Iterator {
 public:
  Iterator() {};
  virtual ~Iterator() {};
  virtual bool HasNext() { return false; }
  virtual Status Next(Log &key) { return Status::OK(); }
};

class Compactor {
 public:
  virtual Status compact() = 0;
};

class Flusher {
 public:
  virtual ~Flusher() {};
  virtual Status Init() = 0;
  virtual Status Stop() = 0;
  virtual Status Flush(const std::string &dir, const std::string &filename,
               Iterator &mem_iter, const Log &last_log, SStable &sst) = 0;
};

class Store {
 public:
  Store() {};
  virtual ~Store() {};
  virtual Status Init(const std::string& path) = 0;
  virtual Status Start() = 0;
  virtual Status Stop() = 0;
  virtual Status Put(const std::string &key, const std::string &value) = 0;
  virtual Status Get(const std::string &key, std::string &value) = 0;
  virtual Status Delete(const std::string &key) = 0;
  // virtual Iterator Query(const std::string &Prefix) = 0;
};

}  // namespace wal
}  // namespace ctgfs
