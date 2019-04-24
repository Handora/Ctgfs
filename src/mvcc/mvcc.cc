#include "mvcc/mvcc.h"


namespace ctgfs {
namespace mvcc {

SimpleMVCC::SimpleMVCC(const std::shared_ptr<KV> kv) 
  : kv_(kv) {

}

SimpleMVCC::~SimpleMVCC() {

}

bool SimpleMVCC::Insert(const string& key, const string& value, uint64 ts) {

}

bool SimpleMVCC::Select(const string& key, const string& value, uint64 ts) {

}

bool SimpleMVCC::Delete(const string& key, uint64 ts) {

}

}}