// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <util/waiter.h>
#include <bthread/bthread.h>

namespace ctgfs {
namespace util {

TEST(UtilTest, Basic) {
  Waiter waiter;
  waiter.Signal();
  Status s = waiter.Wait(1000);

  EXPECT_EQ(true, s.IsOK());
}

int example_var = 0;

void* waitMain(void* args) {
  bthread_usleep(500 * 1000);

  auto waiter = (Waiter*)(args); 
  waiter->Signal();
  example_var = 10;
  return nullptr;
}

TEST(UtilTest, Concurrency) {
  using namespace bthread;
  Waiter waiter;
  bthread_t tid;
  bthread_start_background(&tid, nullptr, waitMain, &waiter);
  EXPECT_EQ(0, example_var);

  Status s = waiter.Wait(1000);
  EXPECT_EQ(10, example_var);

  EXPECT_EQ(true, s.IsOK());
}

void* waitMain2(void* args) {
  bthread_usleep(500 * 1000);

  auto waiter = (Waiter*)(args); 
  waiter->Signal();
  return nullptr;
}

TEST(UtilTest, Concurrency2) {
  using namespace bthread;
  Waiter waiter;
  bthread_t tid;
  bthread_start_background(&tid, nullptr, waitMain2, &waiter);

  Status s = waiter.Wait(200);

  EXPECT_EQ(false, s.IsOK());
}

}  // namespace util
}  // namespace ctgfs
