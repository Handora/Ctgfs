// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <parser/parser.h>
#include <client.pb.cc>

namespace ctgfs {
namespace parser {

TEST(ParserTest, Basic) {
  Parser parser;
  ctgfs::ClientRequest request;
  parser.ParseFromInput("mkdir /a/b", request);

  EXPECT_EQ("/a/b", request.command().path());
  EXPECT_EQ("", request.command().value());
  EXPECT_EQ(1, request.id());
  EXPECT_EQ("127.0.0.1", request.addr());
  EXPECT_EQ(ClientRequest_Command_Type_kCreateDir, request.command().type());
}

} // ctgfs
} // parser
