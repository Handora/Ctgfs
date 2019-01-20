/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */

#pragma once

#include <string>

namespace ctgfs {
namespace util {

struct Options {
  /******************* Raft Part ********************/

  // A follower would become a candidate if it doesn't receive any message
  // from the leader in |election_timeout_ms| milliseconds
  //
  // Default: 1000 (1s)
  int election_timeout_ms;  // follower to candidate timeout

  // A snapshot saving would be triggered every |snapshot_interval_s| seconds
  // if this was reset as a positive number
  // If |snapshot_interval_s| <= 0, the time based snapshot would be disabled.
  //
  // Default: 3600 (1 hour)
  int snapshot_interval_s;

  // We will regard a adding peer as caught up if the margin between the
  // last_log_index of this peer and the last_log_index of leader is less than
  // |catchup_margin|
  //
  // Default: 1000
  int catchup_margin;

  // Describe a specific LogStorage in format ${type}://${parameters}
  //
  // Default: local://./tmp/log
  std::string log_uri;

  // Describe a specific RaftMetaStorage in format ${type}://${parameters}
  //
  // Default: local://./tmp/raft_meta
  std::string raft_meta_uri;

  // Describe a specific SnapshotStorage in format ${type}://${parameters}
  //
  // Default: local://./tmp/snapshot
  std::string snapshot_uri;

  // Group of the raft replication
  std::string group_id;

  // inital peer conf
  //
  // Default: ""
  std::string initial_conf;

  /****************** General Part ******************/
  // Port of the local server
  uint32_t local_port;

  /****************** Rocksdb Part ******************/
  // rocksdb path
  //
  // Default: /tmp/rocksdb
  std::string rocksdb_path;

  Options(std::string group, uint32_t port);
};

inline Options::Options(std::string group, uint32_t port)
    : election_timeout_ms(1000),
      snapshot_interval_s(3600),
      catchup_margin(1000),
      log_uri("local://./tmp/log"),
      raft_meta_uri("local://./tmp/raft_meta"),
      snapshot_uri("local://./tmp/snapshot"),
      group_id(group),
      local_port(port),
      rocksdb_path("/tmp/rocksdb") {}

}  // namespace util
}  // namespace ctgfs
