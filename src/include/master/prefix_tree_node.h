/*
 * author: fftlover(ltang970618@gmail.com)
**/

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

// this function will be called
// when a dir only has one file
// it will be merged from the bottom to the top
void MergeFileNodeToDirNode(PrefixTreeNodePtr file_node, PrefixTreeNodePtr dir_node);

// this function will be called
// when a dir has two or more file
void SplitFileNode(PrefixTreeNodePtr origin_file_node, PrefixTreeNodePtr cur_file_node, const std::string& prefix);

// abstract base class of tree node
class PrefixTreeNode { 
 public:
  PrefixTreeNode() = default;
  PrefixTreeNode(const PrefixTreeNodePtr);
  virtual ~PrefixTreeNode() = 0;
  // overwrite to determin if the node is a dir
  virtual bool IsDir() = 0;
  virtual std::set<std::shared_ptr<PrefixTreeNode>, decltype(TreeNodeComp)*> GetList();
  virtual PrefixTreeNodePtr InsertNode(PrefixTreeNodePtr);
  virtual PrefixTreeNodePtr EraseNode(PrefixTreeNodePtr);
  virtual unsigned int GetFileAndDirCount() const;
  std::string GetPath() const;
  void SetPath(const std::string&);
  std::shared_ptr<PrefixTreeNode> GetParent() const;
  void SetParent(PrefixTreeNodePtr parent);
  friend void MergeFileNodeToDirNode(PrefixTreeNodePtr file_node, PrefixTreeNodePtr dir_node);
  friend void SplitFileNodeToDirNode(PrefixTreeNodePtr origin_file_node, PrefixTreeNodePtr cur_file_node);
  
 protected:
  // relative path
  // should split
  std::string path_;
  // ptr point to parent dir
  std::shared_ptr<PrefixTreeNode> parent_;
};

class PrefixTreeDirNode: public PrefixTreeNode {
 public:
  PrefixTreeDirNode() = delete;
  PrefixTreeDirNode(const PrefixTreeDirNode&);
  PrefixTreeDirNode(const std::string& path, const PrefixTreeNodePtr parent);
  ~PrefixTreeDirNode() {}
  bool IsDir();
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
  PrefixTreeFileNode(const std::string& path, const PrefixTreeNodePtr parent);
  ~PrefixTreeFileNode() {}
  bool IsDir();
};

} // namespace prefix_tree
} // namespace ctgfs
