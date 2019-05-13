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

// origin     cur
//  A         A\B
//  |
//  B
void MergeFileNodeToDirNode(PrefixTree* t, PrefixTreeNodePtr file_node, PrefixTreeNodePtr dir_node) {
  // if(file_node->IsDir() || !dir_node->IsDir())
  //   return false;
  auto fa = dir_node->GetParent();
  fa->EraseNode(dir_node);
  auto dir_path = dir_node->GetPath();
  file_node->SetPath(std::move(dir_path + "/" + file_node->GetPath()));
  file_node->SetParent(fa); 
  fa->InsertNode(file_node);
  auto domain_id = dir_node->GetDomainId();
  if(domain_id != -1) {
    file_node->SetDomainId(domain_id);
    std::list<PrefixTreeNodePtr>* list_ptr = nullptr;
    for(auto& kv_info : t->kv_list_) {
      if(kv_info.id == domain_id) {
        list_ptr = &(kv_info.domain_node_list);
        break;
      }
    }
    if(list_ptr) {
      for(auto it = (*list_ptr).begin(); it != (*list_ptr).end(); it ++) {
        if((*it).get() == dir_node.get()) {
          (*it).reset(file_node.get());
          break;
        }
      }
    }
  }
}

// insert A\D
// origin  cur
//   A      A
//  / \    / \
// B\C X  B   X 
//       / \
//      C   D
void SplitFileNode(PrefixTree* t, PrefixTreeNodePtr origin_file_node, PrefixTreeNodePtr cur_file_node, const std::string& common_prefix) {
  auto origin_file_path = origin_file_node->GetPath();
  auto cur_file_path = cur_file_node->GetPath();
  // split B\C
  auto fa = origin_file_node->GetParent();
  fa->EraseNode(origin_file_node);
  PrefixTreeNodePtr dir_ptr = std::make_shared<PrefixTreeDirNode>(common_prefix, -1, origin_file_node->GetParent());
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
  dir_ptr->SetSZ(origin_file_node->GetSZ() + cur_file_node->GetSZ());
  // update domain
  auto domain_id = origin_file_node->GetDomainId();
  if(domain_id != -1) {
    origin_file_node->SetDomainId(-1);
    dir_ptr->SetDomainId(domain_id);
    std::list<PrefixTreeNodePtr>* list_ptr = nullptr;
    for(auto& kv_info : t->kv_list_) {
      if(kv_info.id == domain_id) {
        list_ptr = &(kv_info.domain_node_list);
        break;
      }
    }
    if(list_ptr) {
      for(auto it = (*list_ptr).begin(); it != (*list_ptr).end(); it ++) {
        if((*it).get() == origin_file_node.get()) {
          (*it).reset(dir_ptr.get());
          break;
        }
      }
    }
  }
}



PrefixTree::PrefixTree() {
  int kv = -1;
  root_ = createNode("", 0, true, kv);
}

PrefixTreeNodePtr PrefixTree::GetRoot() const {
  return root_;
}

std::vector<std::shared_ptr<AdjustContext> >* PrefixTree::GetAdjustContext() {
  if(has_get_context_)
    return &adjust_context_vec_;
  calculateAdjustContext();
  return &adjust_context_vec_;
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

Status PrefixTree::Create(std::string path, unsigned long long ino, bool is_dir, unsigned long long node_sz, int& kv_id) {
  kv_id = -1;
  return doInsert(root_, ino, path, is_dir, node_sz, kv_id);
}

Status PrefixTree::doInsert(PrefixTreeNodePtr cur_node, unsigned long long ino, std::string& path, bool is_dir, unsigned long long node_sz, int& domain_id) {
  // if node isn't a dir
  // and need visit its child
  // error
  if(!cur_node->IsDir()) {
    return Status::PrefixTreeError();
  }
  if(domain_id == -1) {
    domain_id = cur_node->GetDomainId();
  }
  auto sz = cur_node->GetFileAndDirCount();
  // no child 
  // insert directly
  if(sz == 0) {
    auto child_node = createNode(path, ino, is_dir, domain_id, cur_node, node_sz);
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
    auto gen_path = cur_path;
    if(!path.empty())
      gen_path += "/" + path;
    auto child_node = createNode(gen_path, ino, is_dir, domain_id, cur_node, node_sz);
    pushUp(cur_node);
    return Status::OK();
  }
  else {
    if ((*insert_pos)->GetPath() == cur_path) {
      auto status = doInsert(*insert_pos, ino, path, is_dir, node_sz, domain_id);
      pushUp(cur_node);
      return status;
      // return Status::PrefixTreeError();
    }
    else {
      // can't be split
      if(!status) {
        auto child_node = createNode(cur_path, ino, is_dir, domain_id, cur_node, node_sz);
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
              auto flag = doInsert(*insert_pos, ino, path, is_dir, node_sz, domain_id);
              pushUp(cur_node);
              return flag;
            }

            tmp_status = splitPath(tmp_path, tmp_split_path);
            status = splitPath(path, cur_path);
          }
          auto gen_path = common_prefix + cur_path;
          if(!path.empty()) {
            gen_path += "/" + path;
          }
          auto child_node = createNode(gen_path, ino, is_dir, domain_id, nullptr, node_sz);
          if(domain_id == -1)
            domain_id = (*insert_pos)->GetDomainId();
          // earse last '/'
          common_prefix.pop_back();
          SplitFileNode(this, *insert_pos, child_node, common_prefix);
          // std::string p(cur_path + "/" + path);
          // auto res = doInsert(*insert_pos, p, is_dir);
          pushUp(cur_node);
          return Status::OK();
        }
        else {
          createNode(cur_path + "/" + path, ino, is_dir, domain_id, cur_node, node_sz);
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
          MergeFileNodeToDirNode(this, *list.begin(), cur_node);
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

PrefixTreeNodePtr PrefixTree::createNode(const std::string& path, unsigned long long ino, bool is_dir, int& domain_id, PrefixTreeNodePtr parent, unsigned long long sz) {
  PrefixTreeNodePtr new_node;
  if(is_dir) {
    new_node = std::make_shared<PrefixTreeDirNode>(path, ino, parent);
  }
  else {
    new_node = std::make_shared<PrefixTreeFileNode>(path, ino, parent);
  }
  new_node->SetSZ(sz);
  PrefixTreeNodePtr ptr;
  if(parent != nullptr) {
    if(parent->IsDir()) {
      if(domain_id == -1) {
        const auto& list = parent->GetList();
        auto pos = list.end();
        for(auto it = list.begin();it != list.end(); it++) {
          if((*pos)->GetPath() >= path) {
            break;
          }
          pos = it;
        }
        // auto pos = list.lower_bound(new_node);
        std::pair<int, PrefixTreeNode*> domain_info;
        if(pos == list.end()) {
          // domain_id = (*(list.rbegin()))->GetDomainId();
          domain_info = (*(list.begin()))->FindNearestDomainId(true);
        }
        else {
          // domain_id = (*pos)->GetDomainId();
          domain_info = (*(list.rbegin()))->FindNearestDomainId(false);
        }
        domain_id = domain_info.first;
        auto domain_ptr = domain_info.second;
        new_node->SetDomainId(domain_id);
        for(auto& ele : kv_list_) {
          if(ele.id == domain_id) {
            auto& node_list = ele.domain_node_list;
            auto pos = node_list.end();
            for(auto it = node_list.begin(); it != node_list.end(); it ++) {
              // if((*it)->GetPath() > path) {
              if((*it).get() == domain_ptr) {
                break;
              }
              pos = it;
            }
            if(pos == node_list.end()) {
              node_list.push_front(new_node);
            }
            else {
              node_list.insert(pos, new_node);
            }
            break;
          }
        }
      }
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

void PrefixTree::calculateAdjustContext() {
  if(has_get_context_ == false)
    return;
  for(auto& ele : kv_list_) {
    auto ptr = std::make_shared<AdjustContext>();
    ptr->info = &ele;
    ptr->has_been_push_front = false;
    ptr->cur_memory = calculateSum(ptr->info);
    ptr->score = calculateScore(ptr->cur_memory, (ptr->info)->sum_memory);
    adjust_context_vec_.push_back(ptr);
  }
  has_get_context_ = true;
}

std::vector<std::pair<unsigned long long, PrefixTree::move_t> > PrefixTree::Adjust(int target_score) {
  std::vector<std::shared_ptr<AdjustContext> > context_list = *GetAdjustContext();
  std::vector<std::pair<unsigned long long, move_t> >op_res;
  for(int i = 0;i < (int)context_list.size(); i ++) {
    auto context = context_list[i];
    // std::cout << "context " << context->info->id << " score: " << context->score << std::endl;
    if(context->info->overweighed_tag_cnt > 0) {
      // if possible  must move at least once
      std::shared_ptr<AdjustContext> pre_context_ptr;
      if(i - 1 >= 0)
        pre_context_ptr = context_list[i - 1];
      // else pre_context_ptr = *(context_list.rbegin());
      else goto pull;
      auto mv_ele = *((pre_context_ptr->info->domain_node_list).rbegin());
      do {
        context->info->overweighed_tag_cnt --;
        auto& domain_li = pre_context_ptr->info->domain_node_list;
        auto mv_sz = mv_ele->GetSZ();
        if(mv_sz + context->cur_memory <= context->info->sum_memory) {
          move_t move_addr = std::make_pair(pre_context_ptr->info->id, context->info->id);
          moveSubTree(mv_ele, move_addr, op_res);
          pre_context_ptr->cur_memory -= mv_sz;
          context->cur_memory += mv_sz;
          pre_context_ptr->score = calculateScore(pre_context_ptr->cur_memory, pre_context_ptr->info->sum_memory);
          context->score = calculateScore(context->cur_memory, context->info->sum_memory);
          domain_li.pop_back();
          context->info->domain_node_list.push_front(mv_ele);
          if(pre_context_ptr->score >= context->score) {
            context->info->overweighed_tag_cnt = 0;
            break;
          }
        }
        else {
          auto split_status = splitFileNode(domain_li, mv_ele);
          if(!split_status) {
            context->info->overweighed_tag_cnt = 0;
          }
        }
      }while(context->info->overweighed_tag_cnt > 0);
    }
pull:
    // pull from next
    int standard_score = std::min(LIMIT_SCORE, target_score);
    if(context->score <= standard_score) {
      std::shared_ptr<AdjustContext> nxt_context_ptr;
      if(i + 1 < (int)context_list.size()) 
        nxt_context_ptr = context_list[i + 1];
      // else nxt_context_ptr = context_list[0];
      else goto push;
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
            move_t move_addr = std::make_pair(nxt_context_ptr->info->id, context->info->id);
            moveSubTree(mv_ele, move_addr, op_res);
          }
          else {
            auto split_status = splitFileNode(nxt_context_ptr->info->domain_node_list, mv_ele);
            if(!split_status) {
              break;
            }
          }
        }while(context->score <= standard_score);
      }
    }
push:
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
        // else nxt_context_ptr = context_list[0];
        else break;
        auto nxt_score_after_move = calculateScore(sz + nxt_context_ptr->cur_memory, nxt_context_ptr->info->sum_memory);
        if(nxt_score_after_move <= target_score) {
          move_t move_addr = std::make_pair(context->info->id, nxt_context_ptr->info->id);
          moveSubTree(move_ele, move_addr, op_res);
          // op_res.push_back(std::make_pair(move_ele->GetIno(), move_addr);
          nxt_context_ptr->cur_memory += sz;
          context->cur_memory -= sz;
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
  has_get_context_ = false;
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

void PrefixTree::RegistNewKV(int id, unsigned long long sum_memory) {
  KVInfo info;
  info.id = id;
  info.sum_memory = sum_memory;
  if(kv_list_.empty()) {
    info.domain_node_list.push_back(root_);
    root_->SetDomainId(id);
    kv_list_.push_back(info);
    return;
  }
  if(kv_list_.size() == 1) {
    kv_list_.push_back(info);
    return;
  }
  auto nxt_pos = kv_list_.begin();
  int max_res = 0;
  decltype(nxt_pos) solve_pos;
  for(auto it = kv_list_.begin(); it != kv_list_.end(); it ++) {
    nxt_pos ++;
    if(nxt_pos->overweighed_tag_cnt > max_res) {
      max_res = nxt_pos->overweighed_tag_cnt;
      solve_pos = it;
    }
  }
  if(max_res != 0) {
    kv_list_.insert(solve_pos, info);
  }
  else {
    kv_list_.push_back(info);
  }
}

} // namespace prefix_tree
} // namespace ctgfs
