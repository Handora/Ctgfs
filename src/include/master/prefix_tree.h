/*
** author: fftlover(ltang970618@gmail.com)
*/
#pragma once
#include <master/prefix_tree_node.h>
#include <string>
#include <util/status.h>
#include <memory>
#include <list>
#include <vector>
#include <utility>
#include <map>


namespace ctgfs {
namespace prefix_tree {

using namespace ctgfs::util;
using PrefixTreeNodePtr = std::shared_ptr<PrefixTreeNode>;

// limit params of adjust
const int LIMIT_SCORE = 30;

struct KVInfo{
  // register id on master
  int id;
  // sum of disk memory
  unsigned long long sum_memory;
  // tag
  int overweighed_tag_cnt = 0;
  int lessweighed_tag_cnt = 0;
  std::list<PrefixTreeNodePtr> domain_node_list;
};

struct AdjustContext {
  bool has_been_push_front = false;
  KVInfo* info;
  unsigned long long cur_memory;
  int score;
};

// prefixTree needs at least one kv has regist to the master
class PrefixTree {
 public:
  // source, dst
  typedef std::pair<int,int> move_t;
  PrefixTree();
  PrefixTreeNodePtr GetRoot() const;
  // absolute path
  // create a new dir or file node
  Status Create(std::string path, unsigned long long inum, bool is_dir, unsigned long long sz);
  // Remove a file or dir
  // is dir has sub things will failed
  Status Remove(std::string path);
  // Remove recursively
  Status RemoveTree(std::string path);
  std::vector<std::pair<unsigned long long, move_t> > Adjust(int );
 private:
  PrefixTreeNodePtr root_;
  std::list<KVInfo> kv_list_; 
  // will split param full_path 
  // if full path is A/B/C
  // after split full path will be B/C and split path will be filled with A
  // if can be split return true
  // else return false
  bool splitPath(std::string& full_path, std::string& split_path);
  // create a new tree node and link to parent
  PrefixTreeNodePtr createNode(const std::string& path, unsigned long long ino, bool is_dir, PrefixTreeNodePtr parent = nullptr, unsigned long long sz = 0);
  Status doInsert(PrefixTreeNodePtr node, unsigned long long ino, std::string& path, bool is_dir, unsigned long long sz);
  Status doRemove(PrefixTreeNodePtr node, std::string& path);
  // update info from bottom to top
  void pushUp(PrefixTreeNodePtr node);
  // update the domain of node
  void pushDown(PrefixTreeNodePtr node);
  int calculateScore(KVInfo* info);
  int calculateScore(unsigned long long, unsigned long long);
  unsigned long long calculateSum(KVInfo* info);
  bool splitFileNode(std::list<PrefixTreeNodePtr>& li, PrefixTreeNodePtr node);
  void moveSubTree(PrefixTreeNodePtr node, move_t addr, std::vector<std::pair<unsigned long long, move_t> >& vec);
  void doMove(PrefixTreeNodePtr node, move_t addr, std::vector<std::pair<unsigned long long, move_t> >& vec);
};


} // prefix_tree 
} // ctgfs
