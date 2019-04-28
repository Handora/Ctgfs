// yfs client.  implements FS operations using extent and lock server
#include "client/yfs_client.h"
#include "client/extent_client.h"
#include "client/extent_client_cache.h"
#include "client/lock_client.h"
#include "client/lock_client_cache.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace ctgfs::client;

void
yfs_client::print_hex(const std::string& content) {
  for (unsigned int i = 0; i < content.size(); i++) {
    printf("%x", content[i] & 0xff);
  }
  printf("\n");
}

yfs_client::inum
yfs_client::str_to_inum(std::string k) {
  return *(inum*)(k.c_str());
}

std::string
yfs_client::inum_to_str(inum k) {
  return std::string((char*)(&k), sizeof(inum));
}

int
yfs_client::str_to_int(std::string k) {
  return *(int*)(k.c_str());
}

std::string
yfs_client::int_to_str(int k) {
  return std::string((char*)(&k), sizeof(int));
}

yfs_client::inum
yfs_client::gen_inum(bool is_dir) {
  inum id = 0;
  id += random() % (2147483648); // 2 ^ (32 - 1)
  if (!is_dir)
    id |= 0x80000000;
  printf("gen inum %016llx\n", id);
  return id;
}

int
yfs_client::parse_dir(const std::string& dir_content, std::vector<dirent>& vec) {
  unsigned int i = 0;
  while (i < dir_content.size()) {
    // parse file name
    if (dir_content.size() - i < sizeof(int)) {
      return -1;
    }
    int size = str_to_int(dir_content.substr(i, sizeof(int)));
    i+= sizeof(int);

    if (dir_content.size() - i < (unsigned int)size) {
      return -1;
    }
    std::string name = dir_content.substr(i, size);
    i+= size;

    // parse inum
    if (dir_content.size() - i < sizeof(inum)) {
      return -1;
    }
    inum id = str_to_inum(dir_content.substr(i, sizeof(inum)));
    i += sizeof(inum);

    dirent entry;
    entry.name = name;
    entry.inum = id;
    vec.push_back(entry);
  }

  return 0;
}

int
yfs_client::encode_dir(std::string& content, const std::vector<dirent>& vec) {
  content.clear();
  for (unsigned int i = 0; i < vec.size(); i++) {
    int size = (int)(vec[i].name.size());
    content += int_to_str(size) + vec[i].name + inum_to_str(vec[i].inum);
  }

  return 0;
}

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst): lock_dst_(lock_dst)
{
  // ec = new extent_client_cache(extent_dst);
  // lock_release_user *lu = new lock_release_user_impl(ec);
  ec = nullptr;
  // lc = new lock_client_cache(lock_dst, lu);
  lc = nullptr;
  client = new Client;
}

yfs_client::inum
yfs_client::n2i(std::string n)
{
  std::istringstream ist(n);
  unsigned long long finum;
  ist >> finum;
  return finum;
}

std::string
yfs_client::filename(inum inum)
{
  std::ostringstream ost;
  ost << inum;
  return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
  if(inum & 0x80000000)
    return true;
  return false;
}

bool
yfs_client::isdir(inum inum)
{
  return ! isfile(inum);
}

int
yfs_client::setattr(inum inum, const struct stat* attr, struct stat& st) {
  int r = OK;
  
  initExtentClient(inum);
  lock_protocol::status s = lc->acquire(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  printf("setattr %016llx\n", inum);
  extent_protocol::attr a;
  a.size = attr->st_size;
  if (ec->setattr(inum, a) != extent_protocol::OK) {
    r = IOERR;
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  printf("   setattr %016llx -> sz %u\n", inum, a.size);

  fileinfo info;
  r = getfile(inum, info);
  if (r != OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }
  st.st_mode = S_IFREG | 0666;
  st.st_nlink = 1;
  st.st_atime = info.atime;
  st.st_mtime = info.mtime;
  st.st_ctime = info.ctime;
  st.st_size = info.size;

  s = lc->release(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  return OK;
}

int
yfs_client::getfileInfo(inum inum, fileinfo &fin) {
  initExtentClient(inum);
  lock_protocol::status s = lc->acquire(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  int r = getfile(inum, fin);
  s = lc->release(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  return r;
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
  int r = OK;
  // You modify this function for Lab 3
  // - hold and release the file lock

  printf("getfile %016llx\n", inum);
  extent_protocol::attr a;
  if (ec->getattr(inum, a) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }

  fin.atime = a.atime;
  fin.mtime = a.mtime;
  fin.ctime = a.ctime;
  fin.size = a.size;
  printf("getfile %016llx -> sz %llu\n", inum, fin.size);

 release:

  return r;
}

int
yfs_client::getdirInfo(inum inum, dirinfo &din) {
  initExtentClient(inum);
  lock_protocol::status s = lc->acquire(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  int r = getdir(inum, din);
  s = lc->release(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
  int r = OK;
  // You modify this function for Lab 3
  // - hold and release the directory lock

  printf("getdir %016llx\n", inum);
  extent_protocol::attr a;
  if (ec->getattr(inum, a) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }
  din.atime = a.atime;
  din.mtime = a.mtime;
  din.ctime = a.ctime;

 release:
  return r;
}

int
yfs_client::create(inum parent, std::string name, inum& inum, fileinfo& info)
{
  int r = OK;
  
  initExtentClient(parent);
  lock_protocol::status s = lc->acquire(parent);
  if (s != lock_protocol::OK) {
    return IOERR;
  }

  printf("   create get content in the parent dir %016llx\n", parent);
  std::string data;
  r = get(parent, data);
  if(r != OK) {
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }
  
  printf("   create check %s whether in the parent dir %016llx\n", name.c_str(), parent);
  std::vector<dirent> files;
  if (parse_dir(data, files) < 0) {
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return DIRENTERR;
  }

  for (unsigned int i = 0; i < files.size(); i++) {
    if (files[i].name == name) {
      s = lc->release(parent);
      if (s != lock_protocol::OK) {
        return IOERR;
      }
      return EXIST;
    }
  }

  // inum = gen_inum(false);
  auto res = client->GetInumByName(name, false);
  inum = res.first;
  std::string addr = res.second;
  initExtentClient(addr);
  s = lc->acquire(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  printf("   create create empty %s(%016llx)\n", name.c_str(), inum);
  r = put(inum, "");
  if (r != OK) {
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  printf("   create insert entry %s(%016llx) in the parent dir %016llx\n", name.c_str(), inum, parent);
  dirent f;
  f.name = name;
  f.inum = inum;
  files.push_back(f);
  encode_dir(data, files);
  r = put(parent, data);
  if (r != OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  printf("   create get attr for %s(%016llx)\n", name.c_str(), inum);
  r = getfile(inum, info);
  if (r != OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  s = lc->release(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  s = lc->release(parent);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  return OK;
}

int
yfs_client::put(inum inum, const std::string& value)
{
  int r = OK;

  printf("put %016llx\n", inum);
  if (ec->put(inum, value) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }

 release:
  return r;
}

int
yfs_client::get(inum inum, std::string& value)
{
  int r = OK;

  printf("get %016llx\n", inum);
  if (ec->get(inum, value) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }

 release:
  return r;
}

int
yfs_client::remove(inum inum)
{
  int r = OK;

  printf("remove %016llx\n", inum);

  if (ec->remove(inum) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }

 release:
  return r;
}

int
yfs_client::read(inum inum, size_t size, off_t off, std::string& buf)
{
  buf.clear();
  int r = OK;

  initExtentClient(inum);
  lock_protocol::status s = lc->acquire(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  printf("read %016llx\n", inum);
  r = get(inum, buf);
  if (r != OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  size_t buf_size = buf.size();
  if ((unsigned int)off >= buf_size) {
    printf("   read offset is beyond content of inum[%016llx]\n", inum);
    buf.resize(0);
  } else {
    printf("   read offset is within content of inum[%016llx]\n", inum);
    if (buf_size - off < size) {
      size = buf_size - off;
    }

    buf = std::string(buf.begin() + off, buf.begin() + off + size);
  }

  s = lc->release(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  return OK;
}

int
yfs_client::write(inum inum, size_t size, off_t off, const std::string& buf)
{
  int r = OK;

  initExtentClient(inum);
  lock_protocol::status s = lc->acquire(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  printf("write %016llx\n", inum);

  std::string origin;
  r = get(inum, origin);
  if (r != OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  unsigned int old_size = origin.size();
  if ((unsigned int)off >= old_size) {
    printf("   write write has hools in the content inum[%016llx]\n", inum);
    origin += std::string((unsigned int)off - old_size, '\0');
    origin += buf;
  } else {
    printf("   write write in the content inum[%016llx]\n", inum);
    std::string need_append = "";
    if ((unsigned int)(origin.end() - (origin.begin() + off)) > size) {
      need_append = std::string(origin.begin() + off + size, origin.end());
    }
    origin = std::string(origin.begin(), origin.begin() + off) + buf + need_append;
  }

  r = put(inum, origin);
  if (r != OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  s = lc->release(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  return OK;
}

int
yfs_client::lookup(inum parent, std::string name, inum& inum, bool& is_dir, fileinfo& finfo, dirinfo& dinfo)
{
  int r = OK;
  bool found = false;
  initExtentClient(parent);
  lock_protocol::status s = lc->acquire(parent);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  printf("   lookup try to get content of %s on dir %016llx\n", name.c_str(), parent);
  std::string data;
  r = get(parent, data);
  printf("  get ok with %d\n", r);
  if(r != OK) {
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }
  
  std::vector<dirent> files;
  if (parse_dir(data, files) < 0) {
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  for (unsigned int i = 0; i < files.size(); i++) {
    if (files[i].name == name) {
      found = true;
      inum = files[i].inum;
      printf("   lookup get attr of %s(%016llx) on parent dir %016llx\n", name.c_str(), files[i].inum,  parent);
      if(isfile(inum)) {
        is_dir = false;
        r = getfileInfo(inum, finfo);
        if(r != OK) {
          s = lc->release(parent);
          if (s != lock_protocol::OK) {
            return IOERR;
          }
          return r;
        }
      } else {
        is_dir = true;
        r = getdirInfo(inum, dinfo);
        if(r != OK) {
          s = lc->release(parent);
          if (s != lock_protocol::OK) {
            return IOERR;
          }
          return r;
        }
      }
      break;
    }
  }

  s = lc->release(parent);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  if (found) {
    return OK;
  } else {
    return NOENT;
  } 
}

int
yfs_client::readdir(inum inum, size_t size, off_t off, std::vector<dirent>& files)
{
  int r = OK;
  if(!isdir(inum)){
    return NOTDIR;
  }
  initExtentClient(inum);
  lock_protocol::status s = lc->acquire(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  printf("   readdir from ino[%016llx]\n", inum);
  std::string data;
  r = get(inum, data);
  if(r != yfs_client::OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }
  
  if (parse_dir(data, files) < 0) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return DIRENTERR;
  }

  s = lc->release(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  return OK;
}

int
yfs_client::mkdir(inum parent, std::string name, inum& inum, dirinfo& info)
{
  int r = OK;
  initExtentClient(parent);
  lock_protocol::status s = lc->acquire(parent);
  if (s != lock_protocol::OK) {
    return IOERR;
  }

  printf("   mkdir get content of parent dir %016llx\n", parent);
  std::string data;
  r = get(parent, data);
  if(r != OK) {
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }
  
  printf("   mkdir check %s whether in the parent dir %016llx\n", name.c_str(), parent);
  std::vector<dirent> files;
  if (parse_dir(data, files) < 0) {
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return DIRENTERR;
  }

  for (unsigned int i = 0; i < files.size(); i++) {
    if (files[i].name == name) {
      s = lc->release(parent);
      if (s != lock_protocol::OK) {
        return IOERR;
      }
      return EXIST;
    }
  }

  // inum = gen_inum(true);
  auto res = client->GetInumByName(name, true);
  inum = res.first;
  initExtentClient(res.second);
  s = lc->acquire(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  printf("  mkdir create empty dir %s(%016llx)\n", name.c_str(), inum);
  r = put(inum, "");
  if (r != OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  printf("   mkdir insert entry %s(%016llx) in the parent dir %016llx\n", name.c_str(), inum, parent);
  dirent f;
  f.name = name;
  f.inum = inum;
  files.push_back(f);
  encode_dir(data, files);
  r = put(parent, data);
  if (r != yfs_client::OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  printf("   mkdir get attr for %s(%016llx)\n", name.c_str(), inum);
  r = getdir(inum, info);
  if (r != OK) {
    s = lc->release(inum);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return r;
  }

  s = lc->release(inum);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  s = lc->release(parent);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  return OK;
}

int
yfs_client::unlink(inum parent, std::string name)
{
  int r = OK;
  std::string data;
  initExtentClient(parent);
  lock_protocol::status s = lc->acquire(parent);
  if (s != lock_protocol::OK) {
    return IOERR;
  }

  printf("   unlink %s on parent dir %016llx\n", name.c_str(), parent);
  r = get(parent, data);
  if(r != OK) {
    return r;
  }
  
  printf("   unlink check %s whether in the parent dir %016llx\n", name.c_str(), parent);
  std::vector<dirent> files;
  if (parse_dir(data, files) < 0) {
    s = lc->release(parent);
    if (s != lock_protocol::OK) {
      return IOERR;
    }
    return DIRENTERR;
  }

  inum id = 0;
  bool found = false;
  for (unsigned int i = 0; i < files.size(); i++) {
    if (files[i].name.c_str() == name) {
      found = true;
      if (isdir(files[i].inum)) {
        s = lc->release(parent);
        if (s != lock_protocol::OK) {
          return IOERR;
        }
        return NOENT;
      }
      id = files[i].inum;

      printf("   unlink remove entry %s(%016llx) from the parent dir %016llx\n", name.c_str(), id, parent);
      files.erase(files.begin() + i);
      encode_dir(data, files);
      r = put(parent, data);
      if (r != OK) {
        s = lc->release(parent);
        if (s != lock_protocol::OK) {
          return IOERR;
        }
        return r;
      }
      break;
    }
  }

  s = lc->release(parent);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  if (!found) {
    return NOENT;
  }

  printf("   unlink remove file %s(%016llx)\n", name.c_str(), id);
  initExtentClient(id);
  s = lc->acquire(id);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  r = remove(id);
  s = lc->release(id);
  if (s != lock_protocol::OK) {
    return IOERR;
  }
  if (r != OK) {
    return r;
  }

  return OK;
}

void yfs_client::initExtentClient(inum ino) {
  const std::string& addr = client->GetKVAddrByInum(ino);
  initExtentClient(addr);
}

void yfs_client::initExtentClient(const std::string& addr) {
  if(ec != nullptr) {
    const std::string& ec_addr = ec->GetCurAddr();
    if(addr == ec_addr) {
      printf("reuse\n");
      return;
    }
  }
  if(ec == nullptr) {
    ec = new extent_client_cache(addr);
    printf("start lock real\n");
    lock_release = new lock_release_user_impl(ec);
    printf("start lc\n");
    lc = new lock_client_cache(lock_dst_, lock_release);
    printf("succ\n");
  }
  else {
    printf("reconnect\n");
    ec->ConnectTo(addr);
    // lc->ConnectTo(addr);
  }
}

