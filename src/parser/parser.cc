// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include <parser/parser.h>
#include <client.pb.h>
#include <glog/logging.h>

namespace ctgfs {
namespace parser {
bool Parser::ParseFromInput(const std::string& input, Command& command) {
  using namespace std;
  istringstream iss(input);
  vector<string> tokens{istream_iterator<string>{iss},
                        istream_iterator<string>{}};

  if (tokens[0].compare("mkdir") == 0) {
    LOG(INFO) << "Get Token " << tokens[0] << std::endl;
    command.type = kCREATE_DIR;    
    if (tokens.size() != 2) {
      return false;
    }
    command.path = tokens[1];
  } else if (tokens[0].compare("rmdir") == 0) {
    LOG(INFO) << "Get Token " << tokens[0] << std::endl;
    command.type = kREMOVE_DIR;
    if (tokens.size() != 2) {
      return false;
    }
    command.path = tokens[1];
  } else if (tokens[0].compare("read") == 0) {
    LOG(INFO) << "Get Token " << tokens[0] << std::endl;
    command.type = kREAD_FILE;
    if (tokens.size() != 2) {
      return false;
    }
    command.path = tokens[1];
  } else if (tokens[1].compare("write") == 0) {
    LOG(INFO) << "Get Token " << tokens[0] << std::endl;
    command.type = kWRITE_FILE;
    if (tokens.size() != 3) {
      return false;
    }
    command.path = tokens[1];
    command.value = tokens[2];
  } else if (tokens[1].compare("rm") == 0) {
    LOG(INFO) << "Get Token " << tokens[0] << std::endl;
    command.type = kREMOVE_FILE;
    if (tokens.size() != 2) {
      return false;
    }
    command.path = tokens[1];
  } else {
    return false;
  }

  return true;
}

bool Parser::ConvertToProto(const Command& command, ctgfs::ClientRequest& request) {
  // TODO(handora): Generate id and ip
  request.set_id(1);
  request.set_addr("127.0.0.1");

  return true; 
}
}
} // namespace ctgfs
