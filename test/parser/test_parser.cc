// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <parser/parser.h>
#include <client.pb.cc>

namespace ctgfs {
namespace parser {

TEST(ParserTest, Basic) {
  using namespace ctgfs::util;

  Parser parser;
  ctgfs::ClientKVRequest request;
  Status s = parser.ParseFromInput("mkdir /a/b", request);

  EXPECT_EQ(true, s.IsOK());
  EXPECT_EQ("/a/b", request.command().path());
  EXPECT_EQ("", request.command().value());
  EXPECT_EQ(1, request.id());
  EXPECT_EQ("127.0.0.1", request.addr());
  EXPECT_EQ(ClientKVRequest_Command_Type_kCreateDir, request.command().type());
}

}  // namespace parser
}  // namespace ctgfs
