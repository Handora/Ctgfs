// this is the extent cache

#ifndef extent_client_cache_h
#define extent_client_cache_h

#include <string>
#include <map>
#include "extent_protocol.h"
#include "extent_client.h"
#include "lock_client_cache.h"

class extent_client_cache : public extent_client {

 public:
  struct extent {
    unsigned int atime;
    unsigned int mtime;
    unsigned int ctime;
    std::string content;
    bool deleted;
    bool dirty;

    extent(unsigned int a, unsigned int m,
           unsigned int c, std::string v)
      : atime(a), mtime(m), ctime(c), content(v),
        deleted(false), dirty(false) {}
  };

  extent_client_cache(std::string dst);
  ~extent_client_cache();

  extent_protocol::status put(extent_protocol::extentid_t id, std::string);
  extent_protocol::status get(extent_protocol::extentid_t id, std::string &);
  extent_protocol::status getattr(extent_protocol::extentid_t id, extent_protocol::attr &);
  extent_protocol::status remove(extent_protocol::extentid_t id);
  extent_protocol::status setattr(extent_protocol::extentid_t id, extent_protocol::attr &);
  extent_protocol::status flush(extent_protocol::extentid_t id);

 private:
  pthread_mutex_t cache_mu_;
  std::map<extent_protocol::extentid_t, extent*> cache_map_; 
};

#endif 
