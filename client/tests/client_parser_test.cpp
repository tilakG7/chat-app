#include <gtest/gtest.h>
#include "client/client_parser.h"
#include "common/packet.pb.h"

TEST(ClientParser, EncodeRequestRegister) {
    const string kUsername("Tilak");
    string encoded_data = ClientParser::encodeRequestRegister(kUsername);
    
    trial::RequestRegister req;
    ASSERT_FALSE(req.has_username());
    ASSERT_TRUE(req.ParseFromString(encoded_data));
    ASSERT_TRUE(req.has_username());
    ASSERT_EQ(req.username(), kUsername);
}

TEST(ClientParser, EncodeRequestRegisterIss) {
    const string kUsername("Tilak");
    string encoded_data = ClientParser::encodeRequestRegister(kUsername);
    
    trial::RequestRegister req;
    ASSERT_FALSE(req.has_username());
    istringstream iss(encoded_data);
    ASSERT_TRUE(req.ParseFromIstream(&iss));
    ASSERT_TRUE(req.has_username());
    ASSERT_EQ(req.username(), kUsername);
}