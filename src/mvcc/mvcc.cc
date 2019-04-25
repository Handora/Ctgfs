#include "mvcc/mvcc.h"
#include <map>
#include <climits>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>

namespace ctgfs {
namespace mvcc {
namespace func {

std::string ts_to_str(int t) {
  return std::string((char*)(&t), sizeof(int));
}

int str_to_ts (std::string str) {
  return *(int*)(str.c_str());
}

/* binary search to find the biggest value which is smaller than key or equal to key */
/* {1, 3, 4, 7, 9}, key = 6 -> return 2(index) */
int search_version(const std::vector< std::pair<int, int> >& vec, int key) {
  /* if ret == -1, means the ts is smaller than all the timestamp in vector */
  int ret = -1; 

  int lb = 0, ub = vec.size();
  while (lb <= ub) {
    int mid = (lb + ub) / 2;

    if (vec[mid].first <= key) {
      ret = mid;
      lb = mid + 1;
    } else {
      ub = mid - 1;
    }
  }

  return ret;
}

}}}

namespace ctgfs {
namespace mvcc {

SimpleMVCC::SimpleMVCC(const std::shared_ptr<KV> kv) 
  : m_kv_(kv) {

}

SimpleMVCC::~SimpleMVCC() {

}

bool SimpleMVCC::Insert(const std::string& raw_key, const std::string& value, int ts) {
  /* e.g. {key_2, value} , 2 is the timestamp*/
  std::string key = raw_key + "_" + func::ts_to_str(ts);

  m_kv_->Put(key, value);

  return true;
}

bool SimpleMVCC::Select(const std::string& raw_key, std::string& value, int ts) {
  std::map<std::string, std::string> data;
  m_kv_->Query(raw_key, data);

  if (data.empty())  {
    return false;
  }

  std::vector< std::pair<int, int> > data_key;
  std::vector< std::string > data_value;
  int cnt = 0;
  for (const auto & kv : data) {
    /* get the index of '_' */
    std::size_t pos_ = kv.first.find_last_of("_");

    /* push {key, index} */
    data_key.emplace_back(func::str_to_ts(kv.first.substr(pos_ + 1)), cnt++);

    /* push value */
    data_value.emplace_back(kv.second);
  }

  std::sort(data_key.begin(), data_key.end(), 
    [](const std::pair<int, int>& x, const std::pair<int, int>& y) {
      return x.first < y.first;
  });

  /* to find the latest timestamp which is smaller than ts */
  int idx = func::search_version(data_key, ts);

  /* the timestamp is smaller than any other.*/ 
  if (idx < 0) {
    value = "";
    return false;
  } 

  value = data_value[data_key[idx].second];

  return true;
}

bool SimpleMVCC::Delete(const std::string& raw_key, int ts) {
  // TODO: implement this func.
  return true;
}


}}