// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <client.pb.h>
#include <util/status.h>

namespace ctgfs {
namespace parser {

using namespace util;

class Parser {
 public:
  Status ParseFromInput(const std::string& input, ClientKVRequest& request);
};

}  // namespace parser
}  // namespace ctgfs
