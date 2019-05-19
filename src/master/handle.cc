#include <master/handle.h>
#include <stdio.h>
#include <util/util.h>

namespace ctgfs {
namespace lock_server {

handle_mgr mgr;

handle::handle(std::string m) { h = mgr.get_handle(m); }

rpcc *handle::safebind() {
  if (!h) return NULL;
  ScopedLock ml(&h->cl_mutex);
  if (h->del) return NULL;
  if (h->cl) return h->cl;
  sockaddr_in dstsock;
  make_sockaddr(h->m.c_str(), &dstsock);
  rpcc *cl = new rpcc(dstsock);
  int ret;
  // handle class has to tolerate lossy network, since we may test
  // students' lab with RPC_LOSSY=5 from lab 1 to lab 5
  ret = cl->bind();
  if (ret < 0) {
    CTG_WARN("handle_mgr::get_handle bind failure! %s %d\n", h->m.c_str(), ret);
    delete cl;
    h->del = true;
  } else {
    h->cl = cl;
  }
  return h->cl;
}

handle::~handle() {
  if (h) mgr.done_handle(h);
}

handle_mgr::handle_mgr() {
  VERIFY(pthread_mutex_init(&handle_mutex, NULL) == 0);
}

struct hinfo *handle_mgr::get_handle(std::string m) {
  ScopedLock ml(&handle_mutex);
  struct hinfo *h = 0;
  if (hmap.find(m) == hmap.end()) {
    h = new hinfo;
    h->cl = NULL;
    h->del = false;
    h->refcnt = 1;
    h->m = m;
    pthread_mutex_init(&h->cl_mutex, NULL);
    hmap[m] = h;
  } else if (!hmap[m]->del) {
    h = hmap[m];
    h->refcnt++;
  }
  return h;
}

void handle_mgr::done_handle(struct hinfo *h) {
  ScopedLock ml(&handle_mutex);
  h->refcnt--;
  if (h->refcnt == 0 && h->del) delete_handle_wo(h->m);
}

void handle_mgr::delete_handle(std::string m) {
  ScopedLock ml(&handle_mutex);
  delete_handle_wo(m);
}

// Must be called with handle_mutex locked.
void handle_mgr::delete_handle_wo(std::string m) {
  if (hmap.find(m) == hmap.end()) {
    CTG_WARN("handle_mgr::delete_handle_wo: cl %s isn't in cl list\n",
             m.c_str());
  } else {
    struct hinfo *h = hmap[m];
    if (h->refcnt == 0) {
      if (h->cl) {
        h->cl->cancel();
        delete h->cl;
      }
      pthread_mutex_destroy(&h->cl_mutex);
      hmap.erase(m);
      delete h;
    } else {
      h->del = true;
    }
  }
}

}  // lock_server
}  // ctgfs
