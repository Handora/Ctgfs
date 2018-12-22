/*
* author: wfgu(peter.wfgu@gmail.com)
* */

#pragma once

#include <cstring>
#include <array>
#include <functional>
#include <string>

#include <uuid/uuid.h>

namespace ctgfs {
namespace util {

/*
  * The encapsulation of linux uuid.
  * Use util::NewGuid() to generator an unique id. Stringify the Guid type
  * to make things eaiser.
  * Use std::hash<util::Guid>{}("xxx") to calcultae the hash value.
  * */
class Guid {
 public:
  Guid();
  Guid(const unsigned char* bytes);
  
  // Return the stringification of the guid.
  std::string str() const;
  operator std::string() const;

  // Use the stringification of the guid to get the hash value.
  // std::hash<util::Guid>{}("the string you want to hash.");
  std::size_t GetHash() const;

 private:
  std::array<unsigned char, 16> bytes_;
};


} // namespace util
} // namespace ctgfs


namespace ctgfs {
namespace util {

Guid::Guid() : bytes_({0}) {
    
}  

Guid::Guid(const unsigned char* bytes) {
  std::copy(bytes, bytes + 16, std::begin(bytes_));
}

std::string Guid::str() const {
  char one[10], two[6], three[6], four[6], five[14];

  snprintf(one, 10, "%02x%02x%02x%02x",
              bytes_[0], bytes_[1], bytes_[2], bytes_[3]);
  snprintf(two, 6, "%02x%02x",
              bytes_[4], bytes_[5]);
  snprintf(three, 6, "%02x%02x",
              bytes_[6], bytes_[7]);
  snprintf(four, 6, "%02x%02x",
              bytes_[8], bytes_[9]);
  snprintf(five, 14, "%02x%02x%02x%02x%02x%02x",
              bytes_[10], bytes_[11], bytes_[12], bytes_[13], bytes_[14], bytes_[15]);

  const std::string sep("-");
  std::string result(one);
  result += sep + two;
  result += sep + three;
  result += sep + four;
  result += sep + five;

  return result;
}

Guid::operator std::string() const {
  return str();
}

std::size_t Guid::GetHash() const {
  std::hash<std::string> str_hasher;
  return static_cast<std::size_t>(str_hasher(this->str()));
}

// Generate an unique id.
Guid NewGuid() {
  uuid_t id;
  uuid_generate(id);
  return id;
}

} // namespace util
} // namespace ctgfs


// Template specialization for util::Guid.
namespace std {
  template <>
  struct hash<util::Guid> {
    typedef util::Guid argument_type;
    typedef std::size_t result_type;

    result_type operator()(argument_type const& guid) const {
      return guid.GetHash();
    }
  };
} // namespace std

