/*
 * author: fftlover(ltang970618@gmail.com) 
**/

#include <master/prefix_tree.h>
#include <string>
#include <algorithm>

using namespace ctgfs::util;

namespace ctgfs {
namespace prefix_tree {

using PrefixTreeNodePtr = std::shared_ptr<PrefixTreeNode>;

PrefixTree::PrefixTree() {
  root_ = createNode("", 0, true);
  root_->SetDomainId(1);
  auto kv_info_ptr = std::make_shared<KVInfo>();
  kv_info_ptr->id = 1;
  (kv_info_ptr->domain_node_list).push_back(root_);
}

PrefixTreeNodePtr PrefixTree::GetRoot() const {
  return root_;
}

bool PrefixTree::splitPath(std::string& full_path, std::string& split_path) {
  auto split_pos = full_path.find_first_of('/');
  bool flag = true;
  if(split_pos == std::string::npos) {
    flag = false;
    split_path = full_path;
    full_path = "";
  }
  else {
    split_path = std::string(full_path.begin(), full_path.begin() + split_pos);
    full_path.erase(full_path.begin(), full_path.begin() + split_pos + 1);
  }
  return flag;
}

Status PrefixTree::Create(std::string path, unsigned long long ino, bool is_dir, unsigned long long node_sz) {
  return doInsert(root_, ino, path, is_dir, node_sz);
}

Status PrefixTree::doInsert(PrefixTreeNodePtr cur_node, unsigned long long ino, std::string& path, bool is_dir, unsigned long long node_sz) {
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
    auto child_node = createNode(path, ino, is_dir, cur_node, node_sz);
    pushUp(cur_node);
    return Status::OK();
  }
  // split the front
  std::string cur_path;
  bool status = splitPath(path, cur_path);
  auto list = cur_node->GetList();
  auto insert_pos = list.lower_bound(std::make_shared<PrefixTreeFileNode>(cur_path, 0, nullptr));
  // not found
  if(insert_pos == list.end()) {
    auto child_node = createNode(cur_path + "/" + path, ino, is_dir, cur_node, node_sz);
    pushUp(child_node);
    return Status::OK();
  }
  else {
    if ((*insert_pos)->GetPath() == cur_path) {
      auto status = doInsert(*insert_pos, ino, path, is_dir, node_sz);
      pushUp(cur_node);
      return status;
      // return Status::PrefixTreeError();
    }
    else {
      // can't be split
      if(!status) {
        auto child_node = createNode(cur_path, ino, is_dir, cur_node, node_sz);
        pushUp(cur_node);
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
              return doInsert(*insert_pos, ino, path, is_dir, node_sz);
            }

            tmp_status = splitPath(tmp_path, tmp_split_path);
            status = splitPath(path, cur_path);
          }
          auto gen_path = common_prefix + cur_path;
          if(!path.empty()) {
            gen_path += "/" + path;
          }
          auto child_node = createNode(gen_path, ino, is_dir, nullptr, node_sz);
          // earse last '/'
          common_prefix.pop_back();
          SplitFileNode(*insert_pos, child_node, common_prefix);
          std::string p(cur_path + "/" + path);
          // auto res = doInsert(*insert_pos, p, is_dir);
          pushUp(cur_node);
          return Status::OK();
        }
        else {
          createNode(cur_path + "/" + path, ino, is_dir, cur_node, node_sz);
          pushUp(cur_node);
          return Status::OK();
        }
      }
    }
  }
}

Status PrefixTree::Remove(std::string path) {
  return doRemove(root_, path);
}

Status PrefixTree::doRemove(PrefixTreeNodePtr cur_node, std::string& path) {
  if(!cur_node->IsDir()) {
    return Status::PrefixTreeError();
  }
  auto list = cur_node->GetList();
  std::string cur_path;
  auto status = splitPath(path, cur_path);
  auto pos = list.lower_bound(std::make_shared<PrefixTreeFileNode>(cur_path, 0, nullptr));
  if(pos == list.end()) {
    return Status::PrefixTreeError();
  }
  auto target_dir = (*pos)->GetPath();
  std::string tmp_str;
  auto tmp_status = splitPath(target_dir, tmp_str);
  if(tmp_str == cur_path) {
    do {
      if(tmp_status == false && status == false) {
        cur_node->EraseNode(*pos);
        list = cur_node->GetList();
        if(list.size() == 1) {
          MergeFileNodeToDirNode(*list.begin(), cur_node);
        }
        pushUp(cur_node);
        return Status::OK();
      }
      if(tmp_status == false) {
        pushUp(cur_node);
        return doRemove(*pos, path);
      }
      if(status == false) {
        return Status::PrefixTreeError();
      }
      status = splitPath(path, cur_path);
      tmp_status = splitPath(path, cur_path);
    }while(tmp_str == cur_path);
    return Status::PrefixTreeError();
  }
  else {
    return Status::PrefixTreeError();
  }
}

PrefixTreeNodePtr PrefixTree::createNode(const std::string& path, unsigned long long ino, bool is_dir, PrefixTreeNodePtr parent, unsigned long long sz) {
  PrefixTreeNodePtr new_node;
  if(is_dir) {
    new_node = std::make_shared<PrefixTreeDirNode>(path, ino, parent);
  }
  else {
    new_node = std::make_shared<PrefixTreeFileNode>(path, ino, parent);
  }
  if(parent != nullptr) {
    if(parent->IsDir()) {
      parent->InsertNode(new_node);
    }
  }
  return new_node;
}

void PrefixTree::pushUp(PrefixTreeNodePtr node) {
  auto list = node->GetList();
  unsigned long long sz = 0;
  for(auto ele : list) {
    sz += ele->GetSZ();
  }
  node->SetSZ(sz);
}

void PrefixTree::pushDown(PrefixTreeNodePtr node) {

}

std::vector<std::pair<unsigned long long, PrefixTree::move_t> > PrefixTree::Adjust(int target_score) {
  std::vector<std::shared_ptr<AdjustContext> > context_list;
  for(auto& ele : kv_list_) {
    auto ptr = std::make_shared<AdjustContext>();
    ptr->info = &ele;
    ptr->has_been_push_front = false;
    ptr->cur_memory = calculateSum(ptr->info);
    ptr->score = calculateScore(ptr->cur_memory, (ptr->info)->sum_memory);
    context_list.push_back(ptr);
  }
  std::vector<std::pair<unsigned long long, move_t> >op_res;
  for(int i = 0;i < (int)context_list.size(); i ++) {
    auto context = context_list[i];
    if(context->info->overweighed_tag_cnt > 0) {
      // if possible  must move at least once
      std::shared_ptr<AdjustContext> pre_context_ptr;
      if(i - 1 >= 0)
        pre_context_ptr = context_list[i - 1];
      else pre_context_ptr = *(context_list.rbegin());
      do {
        context->info->overweighed_tag_cnt --;
        auto& domain_li = pre_context_ptr->info->domain_node_list;
        auto mv_ele = *((pre_context_ptr->info->domain_node_list).rbegin());
        auto mv_sz = mv_ele->GetSZ();
        if(mv_sz + context->cur_memory <= context->info->sum_memory) {
          move_t move_addr = std::make_pair(pre_context_ptr->info->id, context->info->id);
          moveSubTree(mv_ele, move_addr, op_res);
          pre_context_ptr->cur_memory -= mv_sz;
          context->cur_memory += mv_sz;
          pre_context_ptr->score = calculateScore(pre_context_ptr->cur_memory, pre_context_ptr->info->sum_memory);
          context->score = calculateScore(context->cur_memory, context->info->sum_memory);
          domain_li.pop_back();
          pre_context_ptr->info->domain_node_list.push_front(mv_ele);
          if(pre_context_ptr->score >= context->score) {
            context->info->overweighed_tag_cnt = 0;
            break;
          }
        }
        else {
          context->info->overweighed_tag_cnt = 0;
        }
      }while(context->info->overweighed_tag_cnt > 0);
    }
    // pull from next
    int standard_score = std::min(LIMIT_SCORE, target_score);
    if(context->score <= standard_score) {
      std::shared_ptr<AdjustContext> nxt_context_ptr;
      if(i + 1 < (int)context_list.size()) 
        nxt_context_ptr = context_list[i + 1];
      else nxt_context_ptr = context_list[0];
      if(nxt_context_ptr->score > target_score) {
        do {
          auto mv_ele = *((nxt_context_ptr->info->domain_node_list).begin());     
          auto mv_sz = mv_ele->GetSZ();
          auto cur_score = calculateScore(context->cur_memory + mv_sz, context->info->sum_memory); 
          auto nxt_score = calculateScore(nxt_context_ptr->cur_memory - mv_sz, nxt_context_ptr->info->sum_memory);
          if(cur_score <= target_score) {
            context->cur_memory += mv_sz;
            nxt_context_ptr->cur_memory -= mv_sz;
            context->score = cur_score;
            nxt_context_ptr->score = nxt_score;
            (nxt_context_ptr->info->domain_node_list).pop_front();
            (context->info->domain_node_list).push_back(mv_ele);
          }
          else {
            break;
          }
        }while(context->score <= standard_score);
      }
    }
    // push to next
    if(context->score > target_score) {
      auto& node_list = (context->info)->domain_node_list;
      while(node_list.size() > 0 && context->score > target_score) {
        auto move_ele = *node_list.rbegin();
        // try move
        auto sz = move_ele->GetSZ();
        std::shared_ptr<AdjustContext> nxt_context_ptr;
        if(i + 1 < (int)context_list.size()) 
          nxt_context_ptr = context_list[i + 1];
        else nxt_context_ptr = context_list[0];
        auto nxt_score_after_move = calculateScore(sz + nxt_context_ptr->score, nxt_context_ptr->info->sum_memory);
        if(nxt_score_after_move <= target_score) {
          move_t move_addr = std::make_pair(context->info->id, nxt_context_ptr->info->id);
          moveSubTree(move_ele, move_addr, op_res);
          // op_res.push_back(std::make_pair(move_ele->GetIno(), move_addr);
          nxt_context_ptr->cur_memory += nxt_score_after_move;
          context->cur_memory -= nxt_score_after_move;
          nxt_context_ptr->score = calculateScore(nxt_context_ptr->cur_memory, nxt_context_ptr->info->sum_memory);
          context->score = calculateScore(context->cur_memory, context->info->sum_memory);
          (context->info->domain_node_list).pop_back();
          (nxt_context_ptr->info->domain_node_list).push_front(move_ele);
        }
        else {
          auto split_status = splitFileNode(node_list, move_ele); 
          if(!split_status) {
            nxt_context_ptr->info->overweighed_tag_cnt ++;
            break;
          }
        }
      }
    }
  }
  std::reverse(op_res.begin(), op_res.end());
  return op_res;
}

int PrefixTree::calculateScore(KVInfo* info) {
  unsigned long long cur_sum = calculateSum(info); 
  return calculateScore(cur_sum, info->sum_memory);
  // return (100 * cur_sum) / info.sum_memory;
}

int PrefixTree::calculateScore(unsigned long long cur_sum, unsigned long long all_sum) {
  return (100 * cur_sum) / all_sum;
}

unsigned long long PrefixTree::calculateSum(KVInfo* info) {
  unsigned long long cur_sum = 0;
  auto domain_node_list = info->domain_node_list;
  for(auto ele : domain_node_list) {
    cur_sum += ele->GetSZ();
  }
  return cur_sum;
}

bool PrefixTree::splitFileNode(std::list<PrefixTreeNodePtr>& domain_li, PrefixTreeNodePtr node) {
  if(node->IsDir() == false) {
    return false; 
  }
  domain_li.pop_back();
  auto l = node->GetList();
  node->PushDownDomainTag();
  for(auto& ele : l) {
    domain_li.push_back(ele);
  }
  return true;
}

void PrefixTree::moveSubTree(PrefixTreeNodePtr node, move_t addr, std::vector<std::pair<unsigned long long, move_t> >& vec) {
  node->SetDomainId(addr.second);
  doMove(node, addr, vec);
}

void PrefixTree::doMove(PrefixTreeNodePtr node, move_t addr, std::vector<std::pair<unsigned long long, move_t> >& vec) {
  if(!node->IsDir()) {
    vec.push_back(std::make_pair(node->GetIno(), addr));
    return;
  }
  auto list = node->GetList();
  for(auto ele : list) {
    doMove(ele, addr,vec);
  }
}

} // namespace prefix_tree
} // namespace ctgfs
