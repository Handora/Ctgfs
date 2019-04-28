#include "mvcc/mvcc.h"
#include <algorithm>
#include <climits>
#include <iostream>
#include <map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ctgfs {
namespace mvcc {
namespace func {

std::string ts_to_str(int t) { return std::string((char*)(&t), sizeof(int)); }

int str_to_ts(std::string str) { return *(int*)(str.c_str()); }

/* binary search to find the biggest value which is smaller than key or equal to
 * key */
/* {1, 3, 4, 7, 9}, key = 6 -> return 2(index) */
int search_version(const std::vector<std::pair<int, int> >& vec, int key) {
  /* if ret == -1, means the ts is smaller than all the timestamp in vector */
  int ret = -1;

  int lb = 0, ub = vec.size() - 1;
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
}
}
}

namespace ctgfs {
namespace mvcc {

SimpleMVCC::SimpleMVCC(const std::shared_ptr<KV> kv) : m_kv_(kv) {}

SimpleMVCC::~SimpleMVCC() {}

bool SimpleMVCC::Insert(const std::string& raw_key, const std::string& value,
                        int ts) {
  /* e.g. {key_2, value} , 2 is the timestamp*/
  std::string key = raw_key + "_" + func::ts_to_str(ts);

  m_kv_->Put(key, value);

  return true;
}

bool SimpleMVCC::Select(const std::string& raw_key, std::string& value,
                        int ts) {
  std::map<std::string, std::string> data;
  m_kv_->Query(raw_key, data);

  if (data.empty()) {
    return false;
  }

  std::vector<std::pair<int, int> > data_timestamp;
  std::vector<std::string> data_value;
  std::unordered_set<int> data_del;
  int cnt = 0;
  for (const auto& kv : data) {
    /* check if the file has been delete. */
    int times_ = std::count(kv.first.begin(), kv.first.end(), '_');
    if (times_ <= 0 || times_ > 2) {
      /* handle interal error */
      return false;
    }

    if (times_ == 1) {
      /* get the index of '_timestamp' */
      std::size_t pos_ts = kv.first.find_last_of("_");

      /* push {timestamp, index} */
      data_timestamp.emplace_back(func::str_to_ts(kv.first.substr(pos_ts + 1)),
                                  cnt++);
    } else {
      /* the file was deleted. */
      std::size_t pos_ts = kv.first.find_first_of("_");
      int t = func::str_to_ts(kv.first.substr(pos_ts + 1, sizeof(int)));
      data_timestamp.emplace_back(t, cnt++);
      data_del.insert(t);
    }

    /* push value */
    data_value.emplace_back(kv.second);
  }

  /* Since the key in map is sorted, the vector has no need to be sorted. */
  // std::sort(data_timestamp.begin(), data_timestamp.end(),
  //   [](const std::pair<int, int>& x, const std::pair<int, int>& y) {
  //     return x.first < y.first;
  // });

  /* to find the latest timestamp which is smaller than ts */
  int idx = func::search_version(data_timestamp, ts);

  /* the timestamp is smaller than any other.*/
  if (idx < 0) {
    value = "";
    return false;
  }

  if (data_del.count(data_timestamp[idx].first)) {
    /* the file was deleted. */
    value = "";
    return false;
  }

  value = data_value[data_timestamp[idx].second];

  return true;
}

bool SimpleMVCC::Delete(const std::string& raw_key, int ts) {
  /* check if the file exists. */
  std::map<std::string, std::string> data;
  m_kv_->Query(raw_key, data);

  if (data.empty()) {
    return false;
  }

  /* e.g. {key_2_d, value} , 2 is the timestamp, d is the delete flag. */
  std::string key = raw_key + "_" + func::ts_to_str(ts) + "_d";

  m_kv_->Put(key, "");

  return true;
}
}
}