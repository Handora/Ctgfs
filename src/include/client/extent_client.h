// extent client interface.

#ifndef extent_client_h
#define extent_client_h

#include <map>
#include <string>
#include "fs/extent_protocol.h"
#include "rpc/rpc.h"

using namespace ::ctgfs::server;
using namespace ::ctgfs::rpc;

namespace ctgfs {
namespace client {

class extent_client {
 protected:
  rpcc* cl;
  std::string addr_;
  std::map<std::string, rpcc*> addr_to_rpcc_;

 public:
  extent_client(const std::string& dst);
  virtual ~extent_client();

  std::string GetCurAddr();
  void ConnectTo(const std::string& dst);
  virtual extent_protocol::status get(extent_protocol::extentid_t eid,
                                      std::string& buf);
  virtual extent_protocol::status getattr(extent_protocol::extentid_t eid,
                                          extent_protocol::attr& a);
  virtual extent_protocol::status put(extent_protocol::extentid_t eid,
                                      std::string buf);
  virtual extent_protocol::status remove(extent_protocol::extentid_t eid);
  virtual extent_protocol::status setattr(extent_protocol::extentid_t id,
                                          extent_protocol::attr& a);
};

}  // namespace ctgfs
}  // namespace client

#endif
