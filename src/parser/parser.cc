// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include <parser/parser.h>
#include <client.pb.h>

namespace ctgfs {
namespace parser {
bool Parser::ParseFromInput(const std::string& input, ctgfs::ClientRequest& request) {
  using namespace std;
  istringstream iss(input);
  vector<string> tokens{istream_iterator<string>{iss},
                        istream_iterator<string>{}};

  // TODO(handora): Generate id and ip
  request.set_id(1);
  request.set_addr("127.0.0.1");
  ClientRequest_Command* command = request.mutable_command();

  if (tokens[0].compare("mkdir") == 0) {
    if (tokens.size() != 2) {
      return false;
    }
    command->set_type(ctgfs::ClientRequest_Command_Type_kCreateDir);
    command->set_path(tokens[1]);
  } else if (tokens[0].compare("rmdir") == 0) {
    if (tokens.size() != 2) {
      return false;
    }
    command->set_type(ctgfs::ClientRequest_Command_Type_kRemoveDir);
    command->set_path(tokens[1]);
  } else if (tokens[0].compare("read") == 0) {
    if (tokens.size() != 2) {
      return false;
    }
    command->set_type(ctgfs::ClientRequest_Command_Type_kReadFile);
    command->set_path(tokens[1]);
  } else if (tokens[1].compare("write") == 0) {
    if (tokens.size() != 3) {
      return false;
    }
    command->set_type(ctgfs::ClientRequest_Command_Type_kWriteFile);
    command->set_path(tokens[1]);
    command->set_value(tokens[2]);
  } else if (tokens[1].compare("rm") == 0) {
    if (tokens.size() != 2) {
      return false;
    }
    command->set_type(ctgfs::ClientRequest_Command_Type_kRemoveFile);
    command->set_path(tokens[1]);
  }

  return true;
}

}
} // namespace ctgfs
