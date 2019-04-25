#pragma once

#ifndef __MVCC_H_
#define __MVCC_H_

#include "kv/kv.h"
#include "kv/mock_kv.h"
#include <memory>
#include <string>
#include <cstdint>


namespace ctgfs {
namespace mvcc {

class MVCC {
  public:
    virtual bool Insert(const std::string& key, const std::string& value, int ts) = 0;
    virtual bool Select(const std::string& key, std::string& value, int ts) = 0;
    virtual bool Delete(const std::string& key, int ts) = 0;
} ;

using namespace kv;
class SimpleMVCC : public MVCC {

 public:
  SimpleMVCC(const std::shared_ptr<KV> kv);
  ~SimpleMVCC();
  bool Insert(const std::string& key, const std::string& value, int ts) override;
  bool Select(const std::string& key, std::string& value, int ts) override;
  bool Delete(const std::string& key, int ts) override;

 private:
  std::shared_ptr<KV> m_kv_;
};

}}

#endif