/*
** author: fftlover(ltang970618@gmail.com)
**/

#include <utility>
#include <master/prefix_tree_node.h>

namespace ctgfs {
namespace prefix_tree {

using PrefixTreeNodePtr = std::shared_ptr<PrefixTreeNode>;

bool TreeNodeComp(PrefixTreeNodePtr lhs, PrefixTreeNodePtr rhs) {
  return lhs->GetPath() < rhs->GetPath();
}

// origin     cur
//  A         A\B
//  |
//  B
void MergeFileNodeToDirNode(PrefixTreeNodePtr file_node, PrefixTreeNodePtr dir_node) {
  // if(file_node->IsDir() || !dir_node->IsDir())
  //   return false;
  auto fa = dir_node->GetParent();
  fa->EraseNode(dir_node);
  auto dir_path = dir_node->GetPath();
  file_node->SetPath(std::move(dir_path + "/" + file_node->GetPath()));
  file_node->SetParent(fa); 
  fa->InsertNode(file_node);
}

// insert A\D
// origin  cur
//   A      A
//  / \    / \
// B\C X  B   X 
//       / \
//      C   D
void SplitFileNode(PrefixTreeNodePtr origin_file_node, PrefixTreeNodePtr cur_file_node, const std::string& common_prefix) {
 auto origin_file_path = origin_file_node->GetPath();
 auto cur_file_path = cur_file_node->GetPath();
 // split B\C
 auto fa = origin_file_node->GetParent();
 fa->EraseNode(origin_file_node);
 PrefixTreeNodePtr dir_ptr = std::make_shared<PrefixTreeDirNode>(common_prefix, origin_file_node->GetParent());
 fa->InsertNode(dir_ptr);
 // generate new C D
 auto changed_origin_file_path = std::string(origin_file_path.begin() + common_prefix.size() + 1,origin_file_path.end()); 
 origin_file_node->SetPath(changed_origin_file_path);
 auto changed_cur_file_path = std::string(cur_file_path.begin() + common_prefix.size() + 1, cur_file_path.end());
 cur_file_node->SetPath(changed_cur_file_path);
 // C D link to B 
 origin_file_node->SetParent(dir_ptr);
 cur_file_node->SetParent(dir_ptr);
 // B point to C D
 dir_ptr->InsertNode(origin_file_node);
 dir_ptr->InsertNode(cur_file_node);
}

// Base Node
PrefixTreeNode::PrefixTreeNode(const PrefixTreeNodePtr node){
  path_ = node->GetPath();
  parent_ = node->GetParent(); 
}

PrefixTreeNodePtr PrefixTreeNode::InsertNode(PrefixTreeNodePtr node) {
  PrefixTreeNodePtr ptr;
  ptr.reset();
  return ptr;
}

PrefixTreeNodePtr PrefixTreeNode::EraseNode(PrefixTreeNodePtr node) {
  PrefixTreeNodePtr ptr;
  ptr.reset();
  return ptr;
}

unsigned int PrefixTreeNode::GetFileAndDirCount() const {
  return 0;
}

std::set<PrefixTreeNodePtr, decltype(TreeNodeComp)*> PrefixTreeNode::GetList() {
  std::set<PrefixTreeNodePtr, decltype(TreeNodeComp)*> s;
  return s;
}

std::string PrefixTreeNode::GetPath() const{
  return path_;
}

void PrefixTreeNode::SetPath(const std::string& path) {
  path_ = path;
}

PrefixTreeNodePtr PrefixTreeNode::GetParent() const {
  return parent_;
}

void PrefixTreeNode::SetParent(PrefixTreeNodePtr parent) {
  parent_ = parent;
}

// Dir Node
PrefixTreeDirNode::PrefixTreeDirNode(const PrefixTreeDirNode& node): list_(TreeNodeComp) { 
  path_ = node.GetPath();
  parent_ = node.GetParent();
}

PrefixTreeDirNode::PrefixTreeDirNode(const std::string& path, const PrefixTreeNodePtr parent ): list_(TreeNodeComp) {
  path_ = path;
  parent_ = parent;
}

unsigned int PrefixTreeDirNode::GetFileAndDirCount() const {
  return list_.size();
}

bool PrefixTreeDirNode::IsDir() {
  return true;
}

auto PrefixTreeDirNode::GetList()->decltype(list_) {
  return list_;
}

PrefixTreeNodePtr PrefixTreeDirNode::InsertNode(PrefixTreeNodePtr node) {
  auto pos = list_.find(node);
  if(pos == list_.end()) {
    list_.insert(node);
    return node;
  }
  PrefixTreeNodePtr p;
  return p;
}

PrefixTreeNodePtr PrefixTreeDirNode::EraseNode(PrefixTreeNodePtr node) {
  auto pos = list_.find(node);
  if(pos == list_.end()) {
    PrefixTreeNodePtr p;
    p.reset();
    return p;
  }
  list_.erase(pos);
  return node;
}

// File Node
PrefixTreeFileNode::PrefixTreeFileNode(const PrefixTreeFileNode& node) { 
  path_ = node.GetPath();
  parent_ = node.GetParent();
}

PrefixTreeFileNode::PrefixTreeFileNode(const std::string& path, const PrefixTreeNodePtr parent) {
  path_ = path;
  parent_ = parent;
}

bool PrefixTreeFileNode::IsDir() {
  return false;
}

} // prefix_tree
} // ctgfs
