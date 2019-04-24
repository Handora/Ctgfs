#pragma once

#ifndef __MVCC_H_
#define __MVCC_H_

#include "kv/kv.h"
#include "kv/mock_kv.h"
#include <memory>
#include <cstdint>


namespace ctgfs {
namespace mvcc {

typedef std::uint64_t uint64;

class MVCC {
  public:
    virtual bool Insert(const string& key, const string& value, uint64 ts) = 0;
    virtual bool Select(const string& key, const string& value, uint64 ts) = 0;
    virtual bool Delete(const string& key, uint64 ts) = 0;
} ;

class SimpleMVCC : public MVCC {
 using ctgfs::kv;

 public:
  SimpleMVCC(const std::shared_ptr<KV> kv);
  ~SimpleMVCC();
  bool Insert(const string& key, const string& value, uint64 ts) override;
  bool Select(const string& key, const string& value, uint64 ts) override;
  bool Delete(const string& key, uint64 ts) override;

 private:
  std::shared_ptr<KV> kv_;
};

}}

#endif