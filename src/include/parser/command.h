#include <string>

namespace ctgfs {
namespace parser {

enum CommandType {
  kCREATE_DIR = 0,
  kWRITE_FILE,
  kREMOVE_DIR,
  kREMOVE_FILE,
  kREAD_FILE
};

struct Command {
  CommandType type;
  std::string path;
  std::string value;
};
} // namespace parser
} // namespace ctgfs
