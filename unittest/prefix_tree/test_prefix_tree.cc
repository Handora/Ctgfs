/*
* author: fftlover(ltang970618@gmail.com)
**/

#include <master/prefix_tree.h>
#include <master/prefix_tree_node.h>
#include <util/status.h>
#include <gtest/gtest.h>
#include <iostream>

namespace ctgfs {
namespace prefix_tree {

void debugTree(std::shared_ptr<PrefixTreeNode> t, std::string pre = "") {
  if(!t->IsDir()) {
    return;
  }
  auto list = t->GetList();
  std::cout << pre << " " << t->GetPath() << std::endl;
  for(auto ele : list) {
    debugTree(ele, pre + "-");
  }
}

TEST(PrefixTreeTest, PrefixTree) {
  PrefixTree* t = new PrefixTree(); 
  t->RegistNewKV(1, 1000);
  EXPECT_TRUE(t->GetRoot()->IsDir());
  int kv_id;
  auto status = t->Create("A/B", 1, false, 0, kv_id);
  EXPECT_FALSE(status.IsOK());
  status = t->Create("A", 1, true, 0, kv_id);
  EXPECT_TRUE(status.IsOK());
  status = t->Create("A/B", 2, false, 0, kv_id); 
  auto list = t->GetRoot()->GetList();
  EXPECT_TRUE(status.IsOK());
  EXPECT_EQ(1, list.size());
  auto dir_node = *list.begin();
  EXPECT_TRUE(dir_node->IsDir());
  EXPECT_EQ("A", dir_node->GetPath());
  auto node_b = *((dir_node->GetList()).begin());
  EXPECT_EQ(node_b->GetPath(), "B");
  status = t->Create("A/C", 3, true, 0, kv_id);
  EXPECT_TRUE(status.IsOK());
  list = t->GetRoot()->GetList();
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

  status = t->Remove("A/B");
  EXPECT_TRUE(status.IsOK());
  list = dir_node->GetList();
  auto node_c = *list.begin();
  EXPECT_TRUE(dir_node->IsDir());
  EXPECT_TRUE(node_c->IsDir());
  EXPECT_EQ(node_c->GetPath(), "C");
  EXPECT_EQ(1, (t->GetRoot()->GetList()).size());
  EXPECT_EQ(1, list.size());
}

} // prefix_tree
} // ctgfs
