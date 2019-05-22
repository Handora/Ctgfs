// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <storage/multi_iter.h>
#include <util/util.h>

namespace ctgfs {
namespace storage {

using util::Status;
using sstable::SStable;
using namespace util;

MultiIter::MultiIter(std::vector<Iterator *> &iters) {
  Status ret = Status::OK();
  iters_.resize(iters.size());
  logs_.resize(iters.size());
  int i = 0;

  for (auto it = iters.begin(); ret.IsOK() && it != iters.end(); it++, i++) {
    iters_[i] = iters[i];
    if (iters_[i]->HasNext()) {
      if (!(ret = iters_[i]->Next(logs_[i])).IsOK()) {
        CTG_WARN("next iterator error");
      }
    } else {
      logs_[i].Reset();
    }
  }
}

bool MultiIter::HasNext() {
  bool bool_ret = false;
  for (auto it = logs_.begin(); false == bool_ret && it != logs_.end(); it++) {
    if (it->IsValid()) {
      bool_ret = true;
    }
  }

  return bool_ret;
}

Status MultiIter::Next(Log &log) {
  Status ret = Status::OK();
  bool exist = false;
  Log choosen;
  int i = 0;

  for (auto it = logs_.begin(); ret.IsOK() && it != logs_.end(); it++) {
    if (it->IsValid()) {
      if (!exist) {
        choosen = *it;
      } else if (it->key == choosen.key) {
        if (*it < choosen) {
          choosen = *it;
        }
      } else if (it->key < choosen.key) {
        choosen = *it;
      }
      exist = true;
    }
  }

  for (auto it = logs_.begin(); ret.IsOK() && exist && it != logs_.end();
       it++, i++) {
    if (it->IsValid()) {
      if (it->key == choosen.key) {
        while (logs_[i].IsValid() && logs_[i].key == choosen.key) {
          if (iters_[i]->HasNext()) {
            if (!(ret = iters_[i]->Next(logs_[i])).IsOK()) {
              CTG_WARN("next iter error");
            }
          } else {
            logs_[i].Reset();
          }
        }
      }
    }
  }

  if (ret.IsOK() && !exist) {
    ret = Status::InvalidArgument("not exist next");
  } else if (ret.IsOK()) {
    log = choosen;
  }

  return ret;
}
}
}
