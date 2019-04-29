#include <unordered_map>
#include <list>

namespace ctgfs {
namespace util {

template <typename T, typename W>
class LRUCache {
 public:
  LRUCache(std::size_t capacity) : capacity_(capacity) { }
    
  bool Get(const T& key, W& value) {
    auto it = hs_table_.find(key);
        
    if (it != hs_table_.end()) {
      /* the key is found in the map, update its freq. */ 
      list_.splice(list_.begin(), list_, it->second);
      hs_table_[key] = list_.begin();
      value = it->second->second;
      return true;
    } 

    return false;
  }
    
  bool Put(const T& key, const W& value) {
    auto it = hs_table_.find(key);
        
    if (it != hs_table_.end()) {
      /* the key is found in the hash table, update its value & freq. */
      it->second->second = value;
      list_.splice(list_.begin(), list_, it->second);
      hs_table_[key] = list_.begin();
    } else {
      /* the key isn't found, insert into the hash table, set its freq. */
      list_.push_front({key, value});
      hs_table_[key] = list_.begin();
            
      if (list_.size() > capacity_) {
        hs_table_.erase(list_.back().first);
        list_.pop_back();
      }
    }
    return true;
  }
    
 private:
  std::size_t capacity_;
  std::unordered_map< T, typename std::list< std::pair<T, W> >::iterator > hs_table_;
  std::list< std::pair<T, W> > list_;
};

}}