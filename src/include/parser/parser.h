// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <parser/command.h>
#include <client.pb.h>

namespace ctgfs {
namespace parser {
class Parser {
 public:
  bool ParseFromInput(const std::string& input, Command& command);
  bool ConvertToProto(const Command& command, ctgfs::ClientRequest& request); // proto is coming
};
} // namespace parser
} // namespace ctgfs
