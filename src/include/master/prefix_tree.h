/*
** author: fftlover(ltang970618@gmail.com)
*/

#include <master/prefix_tree_node.h>
#include <util/status.h>
#include <memory>

using namespace ctgfs::util;

namespace ctgfs {
namespace prefix_tree {

using PrefixTreeNodePtr = std::shared_ptr<PrefixTreeNode>;

class PrefixTree {
 public:
  PrefixTreeNodePtr GetRoot() const;
  // absolute path
  // create a new dir or file node
  Status Create(std::string path, bool is_dir);
  // Remove a file or dir
  // is dir has sub things will failed
  Status Remove(std::string path);
  // Remove recursively
  Status RemoveTree(std::string path);
 private:
  PrefixTreeNodePtr root_;
  // will split param full_path 
  // if full path is A/B/C
  // after split full path will be B/C and split path will be filled with A
  // if can be split return true
  // else return false
  bool splitPath(std::string& full_path, std::string& split_path);
  // create a new tree node and link to parent
  PrefixTreeNodePtr createNode(const std::string& path, bool is_dir, PrefixTreeNodePtr parent = nullptr);
  Status doInsert(PrefixTreeNodePtr node, std::string& path, bool is_dir);
  Status doRemove(std::string& path);
  // update info from bottom to top
  void pushUp(PrefixTreeNodePtr node);
};


} // prefix_tree
} // ctgfs
