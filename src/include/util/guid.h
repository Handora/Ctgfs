#pragma once

#include <uuid/uuid.h>
#include <cstring>
#include <hash>
#include <array>
#include <string>

namespace ctgfs {
namespace util {

/*
  * The encapsulation of linux uuid.
  * Use util::NewGuid() to generator an unique id. Stringify the Guid type
  * to make things eaiser.
  * */
class Guid {
 public:
  Guid() : bytes_({0}) {
  }

  Guid(const unsigned char* bytes) {
    std::copy(bytes, bytes + 16, std::begin(bytes_));
  }
  
  // Return the stringification of the guid.
  std::string str() const {
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

  operator std::string() const {
    return str();
  }

  // Use the stringification of the guid to get the hash value.
  std::size_t hash() const {
    return static_cast<std::size_t>(hasher_(str())); 
  }

 private:
  std::hash<std::string> hasher_;
  std::array<unsigned char, 16> bytes_;
};

// Generate an unique id.
Guid NewGuid() {
  uuid_t id;
  uuid_generate(id);
  return id;
}

} // namespace util
} // namespace ctgfs

namespace std {
  // Template specialization for util::Guid.
  template <>
  class hash<util::Guid> {
   public:
    std::size_t operator()(util::Guid const& guid) const {
      std::hash<std::string> str_hasher;
      return static_cast<std::size_t>(str_hasher(guid.str()));
    }
  };
} // namespace std

