/*
 * author: fftlover(ltang970618@gmail.com)
**/
#pragma once
#include <string>
#include <vector>
#include <set>
#include <memory>


namespace ctgfs {
namespace prefix_tree {

class PrefixTreeNode;

using PrefixTreeNodePtr = std::shared_ptr<PrefixTreeNode>;

// for comp two tree node ptr  using their path
bool TreeNodeComp(PrefixTreeNodePtr lhs, PrefixTreeNodePtr rhs);

// abstract base class of tree node
class PrefixTreeNode { 
 public:
  typedef unsigned long long file_sz_t;
  PrefixTreeNode(){};
  PrefixTreeNode(const PrefixTreeNodePtr);
  virtual ~PrefixTreeNode() { };
  // overwrite to determin if the node is a dir
  virtual bool IsDir() = 0;
  virtual void PushDownDomainTag() = 0;
  virtual PrefixTreeNodePtr InsertNode(PrefixTreeNodePtr);
  virtual PrefixTreeNodePtr EraseNode(PrefixTreeNodePtr);
  virtual unsigned int GetFileAndDirCount() const;
  virtual std::set<PrefixTreeNodePtr, decltype(TreeNodeComp)*> GetList();
  std::string GetPath() const;
  void SetPath(const std::string&);
  std::shared_ptr<PrefixTreeNode> GetParent() const;
  void SetParent(PrefixTreeNodePtr parent);
  void SetSZ(file_sz_t sz);
  file_sz_t GetSZ();
  void SetDomainId(int id);
  int GetDomainId();
  unsigned long long GetIno();
  friend void MergeFileNodeToDirNode(PrefixTreeNodePtr file_node, PrefixTreeNodePtr dir_node);
  friend void SplitFileNodeToDirNode(PrefixTreeNodePtr origin_file_node, PrefixTreeNodePtr cur_file_node);
  
 protected:
  // ino
  unsigned long long ino_;
  // domain kv id
  int domain_id_ = -1;
  // total sz of subtree
  file_sz_t sz_ = 0;
  // relative path
  // should split
  std::string path_;
  // ptr point to parent dir
  std::shared_ptr<PrefixTreeNode> parent_;
  void clearDomainTag();
};

class PrefixTreeDirNode: public PrefixTreeNode {
 public:
  PrefixTreeDirNode() = delete;
  PrefixTreeDirNode(const PrefixTreeDirNode&);
  PrefixTreeDirNode(const std::string& path, const unsigned long long ino, const PrefixTreeNodePtr parent);
  ~PrefixTreeDirNode() {}
  bool IsDir();
  void PushDownDomainTag();
  unsigned int GetFileAndDirCount() const;
  std::set<PrefixTreeNodePtr, decltype(TreeNodeComp)*> GetList();
  PrefixTreeNodePtr InsertNode(PrefixTreeNodePtr node);
  PrefixTreeNodePtr EraseNode(PrefixTreeNodePtr node);
 protected:
  std::set<PrefixTreeNodePtr, bool(*)(PrefixTreeNodePtr, PrefixTreeNodePtr)> list_;
};

class PrefixTreeFileNode: public PrefixTreeNode {
 public:
  PrefixTreeFileNode() = delete;
  PrefixTreeFileNode(const PrefixTreeFileNode&);
  PrefixTreeFileNode(const std::string& path, const unsigned long long ino, const PrefixTreeNodePtr parent);
  ~PrefixTreeFileNode() {}
  bool IsDir();
  void PushDownDomainTag();
};

} // namespace prefix_tree
} // namespace ctgfs
