#ifndef yfs_client_h
#define yfs_client_h

#include <string>
#include <vector>
#include "client.h"
#include "extent_client.h"
#include "extent_client_cache.h"
#include "lock_client.h"
#include "lock_protocol.h"

struct stat;
namespace ctgfs {
namespace client {


class lock_release_user_impl : public lock_release_user {
 private:
  extent_client* ec;

 public:
  lock_release_user_impl(extent_client* c) : ec(c) {}
  void dorelease(lock_protocol::lockid_t id) {
    if (ec != nullptr) {
      ((extent_client_cache*)ec)->flush((extent_protocol::extentid_t)id);
    }
  }
  ~lock_release_user_impl() { ec = NULL; };
};

class yfs_client {
  lock_release_user_impl* lock_release;
  extent_client* ec;
  lock_client* lc;
  Client* client;
  std::string lock_dst_;
  std::string default_extent_dst_;

 public:
  typedef unsigned long long inum;
  enum xxstatus { OK, RPCERR, NOENT, IOERR, EXIST, DIRENTERR, NOTDIR };
  typedef int status;

  struct fileinfo {
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirinfo {
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirent {
    std::string name;
    yfs_client::inum inum;
  };

 private:
  static std::string filename(inum);
  static inum n2i(std::string);
  void initExtentClient(inum ino);
  void initExtentClient(const std::string& addr);
  void print_hex(const std::string& content);
  inum str_to_inum(std::string k);
  std::string inum_to_str(inum k);
  int str_to_int(std::string k);
  std::string int_to_str(int k);
  inum gen_inum(bool is_dir);
  int parse_dir(const std::string& dir_content, std::vector<dirent>& vec);
  int encode_dir(std::string& content, const std::vector<dirent>& vec);

 public:
  yfs_client(std::string, std::string);

  bool isfile(inum);
  bool isdir(inum);

  int getfileInfo(inum, fileinfo&);
  int getfile(inum, fileinfo&);
  int getdirInfo(inum, dirinfo&);
  int getdir(inum, dirinfo&);
  int get(inum, std::string&);
  int put(inum, const std::string&);
  int remove(inum);
  int create(inum parent, std::string name, inum& inum, fileinfo& info);
  int setattr(inum inum, const struct stat* attr, struct stat& st);
  int read(inum inum, size_t size, off_t off, std::string& buf);
  int write(inum inum, size_t size, off_t off, const std::string& buf);
  int lookup(inum parent, std::string name, inum& inum, bool& is_dir,
             fileinfo& finfo, dirinfo& dinfo);
  int readdir(inum inum, size_t size, off_t off, std::vector<dirent>& files);
  int mkdir(inum parent, std::string name, inum& inum, dirinfo& info);
  int unlink(inum parent, std::string name);
};

} // namespace client
} // namespace ctgfs

#endif
