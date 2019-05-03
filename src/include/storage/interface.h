// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once

#include <util/status.h>
#include <string>
#include <storage/log.h>

namespace ctgfs {
namespace storage {

using util::Status;
using namespace util;
using namespace std;

template<typename KeyValue>
class Iterator {
 public:
  Iterator() {};
  virtual ~Iterator() {};
  virtual bool HasNext() = 0;
  virtual Status Next(KeyValue &key) = 0;
};

class Compactor {
 public:
  virtual Status compact() = 0;
};

class Flusher {
 public:
  virtual Status Flush(Iterator<Log> &it) = 0;
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
  virtual Iterator<Log> Query(const std::string &Prefix) = 0;
};

}  // namespace wal
}  // namespace ctgfs
