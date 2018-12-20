// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <client.pb.h>

namespace ctgfs {
namespace parser {
class Parser {
 public:
  bool ParseFromInput(const std::string& input, ClientKVRequest& request);
};
} // namespace parser
} // namespace ctgfs
