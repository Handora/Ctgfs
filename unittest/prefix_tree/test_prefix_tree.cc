/*
* author: fftlover(ltang970618@gmail.com)
**/

#include <master/prefix_tree.h>
#include <master/prefix_tree_node.h>
#include <util/status.h>
#include <gtest/gtest.h>

namespace ctgfs {
namespace prefix_tree {

TEST(PrefixTreeTest, PrefixTree) {
  PrefixTree* t = new PrefixTree(); 
  EXPECT_TRUE(t->GetRoot()->IsDir());
  int kv_id;
  auto status = t->Create("A/B", 1, false, 0, kv_id);
  EXPECT_TRUE(status.IsOK());
  EXPECT_EQ(1, (t->GetRoot()->GetList()).size());
  EXPECT_FALSE((*((t->GetRoot()->GetList()).begin()))->IsDir());
  EXPECT_EQ("A/B", (*(t->GetRoot()->GetList()).begin())->GetPath());
  t->Create("A/C", 2, true, 0, kv_id);
  auto list = t->GetRoot()->GetList();
  EXPECT_EQ(1, list.size());
  ASSERT_TRUE(t->GetRoot()->IsDir());
  auto dir_list = (*list.begin())->GetList();
  int cnt = 0;
  for(auto ele : dir_list) {
    cnt ++;
    if(cnt == 1) {
      EXPECT_FALSE(ele->IsDir());
      EXPECT_EQ("B", (ele)->GetPath());
    }
    else {
      EXPECT_TRUE(ele->IsDir());
      EXPECT_EQ("C", ele->GetPath());
    }
  }

  t->Remove("A/B");
  EXPECT_TRUE((*(t->GetRoot()->GetList()).begin())->IsDir());
  EXPECT_EQ("A/C", (*((t->GetRoot())->GetList()).begin())->GetPath());
  EXPECT_EQ(1, (t->GetRoot()->GetList()).size());
}

} // prefix_tree
} // ctgfs
