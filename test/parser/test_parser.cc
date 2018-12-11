// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <parser/parser.h>
#include <parser/command.h>

namespace ctgfs {
namespace parser {

TEST(ParserTest, Basic) {
  Parser parser;
  Command command;
  parser.ParseFromInput("mkdir /a/b", command);
  EXPECT_EQ("/a/b", command.path);
}

} // ctgfs
} // parser
