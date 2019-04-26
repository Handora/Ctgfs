// extent client interface.

#ifndef extent_client_h
#define extent_client_h

#include <string>
#include "extent_protocol.h"
#include "rpc/rpc.h"

class extent_client {
 protected:
  rpcc *cl;
  const std::string addr_;
 public:
  extent_client(std::string dst);

  std::string GetCurAddr();
  virtual extent_protocol::status get(extent_protocol::extentid_t eid, 
			      std::string &buf);
  virtual extent_protocol::status getattr(extent_protocol::extentid_t eid, 
				  extent_protocol::attr &a);
  virtual extent_protocol::status put(extent_protocol::extentid_t eid, std::string buf);
  virtual extent_protocol::status remove(extent_protocol::extentid_t eid);
  virtual extent_protocol::status setattr(extent_protocol::extentid_t id, extent_protocol::attr& a);
};

#endif 

