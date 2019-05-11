/*
 * author: OneDay_(ltang970618@gmail.com)
 **/
#pragma once
#include <gflags/gflags.h>
#include <master.pb.h>
#include <util/status.h>
#include <memory>
#include <string>
#include "rpc/rpc.h"

namespace ctgfs {
namespace heart_beat {

// fs should fill this struct
// and use this struct to construct heart_beat_sender
// the info is as same as heart_beat_proto
struct HeartBeatInfo {
  int type;
  // HeartBeatMessageRequest_HeartBeatType type; 
  std::string addr;
  unsigned long long file_num;
  unsigned long long disk_usage;
  unsigned long long disk_volume;
  // assume deep copy to make data persistent
  //  void operator=(const HeartBeatInfo& heart_beat_info) {
  //    addr = heart_beat_info.addr;
  //  }
};

}  // namespace heart_beat
}  // namespace ctgfs

/* Implement marshall, unmarshall for HeartBeatInfo */
using ctgfs::heart_beat::HeartBeatInfo;
/* type : int, addr: string */ 
/* file_num: uint64 , disk_usage: uint64, disk_volume: uint64 */
marshall& operator<<(marshall &m, const HeartBeatInfo& hbi) {
  m << hbi.type;
  m << hbi.addr;
  m << hbi.file_num;
  m << hbi.disk_usage;
  m << hbi.disk_volume;
  return m;


unmarshall& operator>>(unmarshall &u, HeartBeatInfo& hbi) {
  u >> hbi.type;
  u >> hbi.addr;
  u >> hbi.file_num;
  u >> hbi.disk_usage;
  u >> hbi.disk_volume;
  return u;
}
