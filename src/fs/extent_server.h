// this is the extent server

#ifndef extent_server_h
#define extent_server_h

#include <string>
#include <map>
#include "extent_protocol.h"

class extent_server {

 public:
  struct extent {
    unsigned int atime;
    unsigned int mtime;
    unsigned int ctime;
    std::string content;

    extent(unsigned int a, unsigned int m,
           unsigned int c, std::string v)
      : atime(a), mtime(m), ctime(c), content(v) {}
  };

  extent_server();
  ~extent_server();

  int put(extent_protocol::extentid_t id, std::string, int &);
  int get(extent_protocol::extentid_t id, std::string &);
  int getattr(extent_protocol::extentid_t id, extent_protocol::attr &);
  int remove(extent_protocol::extentid_t id, int &);
  int setattr(extent_protocol::extentid_t id, extent_protocol::attr, int&);

 private:
  pthread_mutex_t server_mu_;
  std::map<extent_protocol::extentid_t, extent*> extent_map_; 
};

#endif 
