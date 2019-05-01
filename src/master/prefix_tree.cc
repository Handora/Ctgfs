/*
 * author: fftlover(ltang970618@gmail.com) 
**/

#include <master/prefix_tree.h>

// using namespace ctgfs::util;

namespace ctgfs {
namespace prefix_tree {

using PrefixTreeNodePtr = std::shared_ptr<PrefixTreeNode>;

PrefixTreeNodePtr PrefixTree::GetRoot() const {
  return root_;
}

bool PrefixTree::splitPath(std::string& full_path, std::string& split_path) {
  auto split_pos = full_path.find_first_of('/');
  bool flag = true;
  if(split_pos == std::string::npos) {
    flag = false;
  }
  split_path = std::string(full_path.begin(), full_path.begin() + split_pos);
  if(flag)
    full_path.erase(full_path.begin() + split_pos + 1, full_path.end());
  else full_path = "";
  return flag;
}

Status PrefixTree::Create(std::string path, bool is_dir) {
  return doInsert(root_, path, is_dir);
}

Status PrefixTree::doInsert(PrefixTreeNodePtr cur_node, std::string& path, bool is_dir) {
  // if node isn't a dir
  // and need visit its child
  // error
  if(!cur_node->IsDir()) {
    return Status::PrefixTreeError();
  }
  auto sz = cur_node->GetFileAndDirCount();
  // no child 
  // insert directly
  if(sz == 0) {
    auto child_node = createNode(path, is_dir, cur_node);
    pushUp(child_node);
    return Status::OK();
  }
  // split the front
  std::string cur_path;
  bool status = splitPath(path, cur_path);
  auto list = cur_node->GetList();
  auto insert_pos = list.lower_bound(std::make_shared<PrefixTreeFileNode>(cur_path, nullptr));
  // not found
  if(insert_pos == list.end()) {
    auto child_node = createNode(cur_path + "/" + path, is_dir, cur_node);
    pushUp(child_node);
    return Status::OK();
  }
  else {
    if ((*insert_pos)->GetPath() == cur_path) {
      return Status::PrefixTreeError();
    }
    else {
      // can't be split
      if(!status) {
        auto child_node = createNode(cur_path, is_dir, cur_node);
        pushUp(child_node);
        return Status::OK();
      }
      else {
        auto tmp_path = (*insert_pos)->GetPath();
        std::string tmp_split_path;
        std::string common_prefix;
        auto tmp_status = splitPath(tmp_path, tmp_split_path);
        // has common
        if(tmp_split_path == cur_path) {
          while(tmp_split_path == cur_path) {
            common_prefix += cur_path + "/";
            if(status == false) {
              return Status::PrefixTreeError();
            }
            // A/B insert A/B/C
            if (tmp_status == false) {
              return doInsert(*insert_pos, path, is_dir);
            }

            tmp_status = splitPath(tmp_path, tmp_split_path);
            status = splitPath(path, cur_path);
          }
          auto child_node = createNode(common_prefix + "/" + cur_path + "/" + path, is_dir, cur_node);
          // earse last '/'
          common_prefix.pop_back();
          SplitFileNode(*insert_pos, child_node, common_prefix);
          std::string p(cur_path + "/" + path);
          auto res = doInsert(*insert_pos, p, is_dir);
          pushUp(cur_node);
          return res;
        }
        else {
          pushUp(createNode(cur_path + "/" + path, is_dir, cur_node));
          return Status::OK();
        }
      }
    }
  }
}

PrefixTreeNodePtr PrefixTree::createNode(const std::string& path, bool is_dir, PrefixTreeNodePtr parent) {
  PrefixTreeNodePtr new_node;
  if(is_dir) {
    new_node = std::make_shared<PrefixTreeDirNode>(path, parent);
  }
  else {
    new_node = std::make_shared<PrefixTreeFileNode>(path, parent);
  }
  if(parent != nullptr) {
    if(parent->IsDir()) {
      parent->InsertNode(new_node);
    }
  }
  return new_node;
}

void PrefixTree::pushUp(PrefixTreeNodePtr node) {

}


} // namespace prefix_tree
} // namespace ctgfs
