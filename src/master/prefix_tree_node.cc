/*
** author: fftlover(ltang970618@gmail.com)
**/

#include <utility>
#include <master/prefix_tree_node.h>

namespace ctgfs {
namespace prefix_tree {

using PrefixTreeNodePtr = std::shared_ptr<PrefixTreeNode>;
typedef unsigned long long file_sz_t;

bool TreeNodeComp(PrefixTreeNodePtr lhs, PrefixTreeNodePtr rhs) {
  return lhs->GetPath() < rhs->GetPath();
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

void PrefixTreeNode::SetSZ(file_sz_t sz) {
  sz_ = sz;
}

file_sz_t PrefixTreeNode::GetSZ() {
  return sz_;
}

void PrefixTreeNode::SetDomainId(int id) {
  domain_id_ = id;
}

int PrefixTreeNode::GetDomainId() {
  return domain_id_;
}

void PrefixTreeNode::clearDomainTag() {
  domain_id_ = -1;
}

unsigned long long PrefixTreeNode::GetIno() {
  return ino_;
}

// Dir Node
PrefixTreeDirNode::PrefixTreeDirNode(const PrefixTreeDirNode& node): list_(TreeNodeComp) { 
  path_ = node.GetPath();
  parent_ = node.GetParent();
}

PrefixTreeDirNode::PrefixTreeDirNode(const std::string& path, const unsigned long long ino, const PrefixTreeNodePtr parent ): list_(TreeNodeComp) {
  path_ = path;
  parent_ = parent;
  ino_ = ino;
}

unsigned int PrefixTreeDirNode::GetFileAndDirCount() const {
  return list_.size();
}

bool PrefixTreeDirNode::IsDir() {
  return true;
}

void PrefixTreeDirNode::PushDownDomainTag() {
  if(domain_id_ == -1)
    return;
  for(auto node : list_) {
    node->SetDomainId(domain_id_);
  }
  clearDomainTag();
}

std::pair<int, PrefixTreeNode*> PrefixTreeDirNode::FindNearestDomainId(bool left) {
  if(domain_id_ != -1)
    return std::make_pair(domain_id_, this);
  if(list_.empty())
    return std::make_pair(-1, this);
  if(left)
    return (*(list_.begin()))->FindNearestDomainId();
  else return (*(list_.rbegin()))->FindNearestDomainId();
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

PrefixTreeFileNode::PrefixTreeFileNode(const std::string& path, const unsigned long long ino, const PrefixTreeNodePtr parent) {
  path_ = path;
  parent_ = parent;
  ino_ = ino;
}

bool PrefixTreeFileNode::IsDir() {
  return false;
}

void PrefixTreeFileNode::PushDownDomainTag() {
  return;
}

std::pair<int, PrefixTreeNode*> PrefixTreeFileNode::FindNearestDomainId(bool left) {
  return std::make_pair(domain_id_, this);
}

} // prefix_tree
} // ctgfs
