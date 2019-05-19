/*
** author: fftlover(ltang970618@gmail.com)
*/
#pragma once
#include <master/prefix_tree_node.h>
#include <util/status.h>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ctgfs {
namespace prefix_tree {

using namespace ctgfs::util;
using PrefixTreeNodePtr = std::shared_ptr<PrefixTreeNode>;
class PrefixTree;

// this function will be called
// when a dir only has one file
// it will be merged from the bottom to the top
// void MergeFileNodeToDirNode(PrefixTree* t, PrefixTreeNodePtr file_node,
// PrefixTreeNodePtr dir_node);

// this function will be called
// when a dir has two or more file
// void SplitFileNode(PrefixTree* t, PrefixTreeNodePtr origin_file_node,
// PrefixTreeNodePtr cur_file_node, const std::string& prefix);

// limit params of adjust
const int LIMIT_SCORE = 30;

struct KVInfo {
  // register id on master
  int id;
  // sum of disk memory
  unsigned long long sum_memory;
  // tag
  int overweighed_tag_cnt = 0;
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
  typedef std::pair<int, int> move_t;
  PrefixTree();
  PrefixTreeNodePtr GetRoot() const;
  // get context
  std::vector<std::shared_ptr<AdjustContext> >* GetAdjustContext();
  // absolute path
  // create a new dir or file node
  Status Create(std::string path, unsigned long long inum, bool is_dir,
                unsigned long long sz, int& kv_id);
  // Remove a file or dir
  // is dir has sub things will failed
  Status Remove(std::string path);
  // Remove recursively
  Status RemoveTree(std::string path);
  std::vector<std::pair<unsigned long long, move_t> > Adjust(int);
  // regist the kv to prefix tree
  void RegistNewKV(int id, unsigned long long sum_memory);
  // friend void MergeFileNodeToDirNode(PrefixTree*, PrefixTreeNodePtr
  // file_node, PrefixTreeNodePtr dir_node);
  // friend void SplitFileNode(PrefixTree*, PrefixTreeNodePtr,
  // PrefixTreeNodePtr, const std::string& prefix);
 private:
  // current adjust has caculated context
  bool has_get_context_ = false;
  PrefixTreeNodePtr root_;
  std::list<KVInfo> kv_list_;
  // stored the context temp
  std::vector<std::shared_ptr<AdjustContext> > adjust_context_vec_;
  // will split param full_path
  // if full path is A/B/C
  // after split full path will be B/C and split path will be filled with A
  // if can be split return true
  // else return false
  bool splitPath(std::string& full_path, std::string& split_path);
  // create a new tree node and link to parent

  PrefixTreeNodePtr createNode(const std::string& path, unsigned long long ino,
                               bool is_dir, int& domain_id,
                               PrefixTreeNodePtr parent = nullptr,
                               unsigned long long sz = 0);
  Status doInsert(PrefixTreeNodePtr node, unsigned long long ino,
                  std::string& path, bool is_dir, unsigned long long sz,
                  int& domain_id);
  Status doRemove(PrefixTreeNodePtr node, std::string& path);
  // update info from bottom to top
  void pushUp(PrefixTreeNodePtr node);
  // update the domain of node
  void pushDown(PrefixTreeNodePtr node);
  int calculateScore(KVInfo* info);
  int calculateScore(unsigned long long, unsigned long long);
  unsigned long long calculateSum(KVInfo* info);
  void calculateAdjustContext();
  bool splitFileNode(std::list<PrefixTreeNodePtr>& li, PrefixTreeNodePtr node);
  void moveSubTree(PrefixTreeNodePtr node, move_t addr,
                   std::vector<std::pair<unsigned long long, move_t> >& vec);
  void doMove(PrefixTreeNodePtr node, move_t addr,
              std::vector<std::pair<unsigned long long, move_t> >& vec);
};

}  // prefix_tree
}  // ctgfs
