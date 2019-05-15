/*
 * author: fftlover(ltang970618@gmail.com)
 */

#include <master/prefix_tree.h>
#include <master/prefix_tree_node.h>
#include <util/status.h>
#include <gtest/gtest.h>
#include <iostream>

namespace ctgfs {
namespace prefix_tree {

void travelTree(std::shared_ptr<PrefixTreeNode> node, int expect_value);

void debugTree(std::shared_ptr<PrefixTreeNode> node, std::string prefix = "-") {
  std::cout << prefix << " " << node->GetPath() << " sz: " << node->GetSZ() << " domain id: " << node->GetDomainId() << std::endl;
  if(!node->IsDir()) {
    return;
  }
  auto list = node->GetList();
  prefix = "  " + prefix;
  for(auto ele : list) {
    debugTree(ele, prefix);
  }
}

// regist kv id: 1 memory: 10 
// regist kv id: 2 memory: 10
// create file path: a/z/a/a inum: 1 sz: 1 
// create file path: a/z/b/b inum: 2 sz: 5
// create file path: a/z/b/c inum: 3 sz: 1
// create file path: a/z/b/d inum: 4 sz: 1
// adjust
// regist kv id: 3 memory: 10
// create file path: a/z/a/b inum: 5 sz: 5
TEST(PrefixTreeTest, PrefixTree) {
  PrefixTree* t = new PrefixTree();
  int kv_id;

  t->RegistNewKV(1, 10);
  t->RegistNewKV(2, 10);
  auto status = t->Create("a", 11, true, 0, kv_id);
  EXPECT_TRUE(status.IsOK());
  status = t->Create("a/z", 12, true, 0, kv_id); 
  EXPECT_TRUE(status.IsOK());
  status = t->Create("a/z/a", 13, true, 0, kv_id);
  EXPECT_TRUE(status.IsOK());
  t->Create("a/z/a/a", 1, false, 1, kv_id);

  EXPECT_EQ((t->GetRoot())->GetDomainId(), 1);
  EXPECT_EQ(kv_id, 1);
  auto list = (t->GetRoot())->GetList();
  EXPECT_EQ(list.size(), 1);
  status = t->Create("a/z/b", 14, true, 0, kv_id);
  EXPECT_TRUE(status.IsOK());
  t->Create("a/z/b/b", 2, false, 5, kv_id);
  EXPECT_EQ((t->GetRoot())->GetDomainId(), 1);
  EXPECT_EQ(kv_id, 1);
  list = (t->GetRoot())->GetList();

  t->Create("a/z/b/c", 3, false, 1, kv_id);
  EXPECT_EQ(kv_id, 1);

  t->Create("a/z/b/d", 4, false, 1, kv_id);
  EXPECT_EQ(kv_id, 1);

  list = (t->GetRoot())->GetList();
  EXPECT_EQ(list.size(), 1);
  for(auto ele : list) {
    travelTree(ele, -1);
  }
  t->Adjust(40);
  list = t->GetRoot()->GetList();
  EXPECT_EQ(t->GetRoot()->GetDomainId(), -1);
  auto a_node = *list.begin();
  list = a_node->GetList();
  auto a_z_node = *list.begin();
  list = a_z_node->GetList();
  auto a_z_a_node = *list.begin();
  auto a_z_b_node = *list.rbegin();
  list = a_z_a_node->GetList();
  auto node1 = *list.begin();
  auto tmp_list = a_z_b_node->GetList();
  auto node2 = *tmp_list.begin();
  auto pos = tmp_list.begin();
  pos ++;
  auto node3 = *pos;
  auto node4 = *tmp_list.rbegin();
  EXPECT_EQ(node1->GetDomainId(), -1);
  EXPECT_EQ(node2->GetDomainId(), 2);
  EXPECT_EQ(node3->GetDomainId(), 2);
  EXPECT_EQ(node4->GetDomainId(), 2);
  t->RegistNewKV(3, 10);
  t->Create("a/z/a/b", 5, false, 4, kv_id);
  EXPECT_EQ(1, kv_id);
  // layer 0
  auto node_0_0 = t->GetRoot();
  auto temp_list = node_0_0->GetList();
  node_0_0 = *temp_list.begin();
  // layer 1
  auto layer_1 = node_0_0->GetList();
  layer_1 = (*layer_1.begin())->GetList();
  ASSERT_EQ(2, layer_1.size());
  auto node_1_0 = *layer_1.begin();
  auto node_1_1 = *layer_1.rbegin();
  // layer 2
  auto layer_2_0 = node_1_0->GetList();
  ASSERT_EQ(2, layer_2_0.size());
  auto layer_2_1 = node_1_1->GetList();
  ASSERT_EQ(3, layer_2_1.size());
  auto node_1 = *(layer_2_0.begin());
  auto node_5 = *(layer_2_0.rbegin());
  auto it = layer_2_1.begin();
  auto node_2 = *(it++);
  auto node_3 = *(it++);
  auto node_4 = *(it);
  EXPECT_EQ(node_1_0->GetDomainId(), 1);
  EXPECT_EQ(node_1->GetDomainId(), -1);
  EXPECT_EQ(node_5->GetDomainId(), -1);
  EXPECT_EQ(node_1_1->GetDomainId(), -1);
  EXPECT_EQ(node_2->GetDomainId(), 2);
  EXPECT_EQ(node_3->GetDomainId(), 2);
  EXPECT_EQ(node_4->GetDomainId(), 2);
  t->Adjust(50);
  EXPECT_EQ(node_1_0->GetDomainId(), 1);
  EXPECT_EQ(node_1->GetDomainId(), -1);
  EXPECT_EQ(node_5->GetDomainId(), -1);
  EXPECT_EQ(node_2->GetDomainId(), 3);
  EXPECT_EQ(node_3->GetDomainId(), 2);
  EXPECT_EQ(node_4->GetDomainId(), 2);
}

void travelTree(std::shared_ptr<PrefixTreeNode> node, int expect_value) {
  auto list = node->GetList();
  EXPECT_EQ(node->GetDomainId(), expect_value);
  for(auto ele : list) {
    travelTree(ele, expect_value);
  }
}

} // prefix_tree
} // ctgfs
