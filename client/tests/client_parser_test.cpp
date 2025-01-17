#include <gtest/gtest.h>
#include "client/client_parser.h"
#include "common/packet.pb.h"

// TEST(ClientParser, EncodeRequestRegister) {
//     const string kUsername("Tilak");
//     string encoded_data = ClientParser::encodeRequestRegister(kUsername);
    
//     trial::RequestRegister req;
//     ASSERT_FALSE(req.has_username());
//     ASSERT_TRUE(req.ParseFromString(encoded_data));
//     ASSERT_TRUE(req.has_username());
//     ASSERT_EQ(req.username(), kUsername);
// }

// TEST(ClientParser, EncodeRequestRegisterIss) {
//     const string kUsername("Tilak");
//     string encoded_data = ClientParser::encodeRequestRegister(kUsername);
    
//     trial::RequestRegister req;
//     ASSERT_FALSE(req.has_username());
//     istringstream iss(encoded_data);
//     ASSERT_TRUE(req.ParseFromIstream(&iss));
//     ASSERT_TRUE(req.has_username());
//     ASSERT_EQ(req.username(), kUsername);
// }

// TEST(Random, TestStr) {
//     string buffer;
//     trial::Header hdr;
//     hdr.set_packet_type(trial::Header::PACKET_TYPE_REQUEST_REGISTER);
    
//     ASSERT_TRUE(hdr.SerializeToString(&buffer));
//     trial::RequestRegister req;
//     req.set_username("BIATCH");
//     ASSERT_TRUE(req.SerializeToString(&buffer));

//     cout << "Header: " << hdr.DebugString() << endl;
//     cout << "Req   : " << req.DebugString() << endl;

//     trial::Header hdr2;
//     trial::RequestRegister req2;
//     istringstream iss(buffer);

//     EXPECT_TRUE(hdr2.ParseFromString(buffer));
//     EXPECT_TRUE(hdr2.has_packet_type());
//     EXPECT_TRUE(req2.ParseFromString(buffer));
//     EXPECT_TRUE(req2.has_username());
//     // ASSERT_TRUE(hdr2.ParseFromIstream(&iss));
//     // ASSERT_TRUE(hdr2.has_packet_type());
//     // ASSERT_TRUE(req2.ParseFromIstream(&iss));
//     // ASSERT_TRUE(req2.has_username());
// }

TEST(Random, Testing2) {
    mcc::Packet packet;
    cout << packet.has_resp_users() << endl;
    cout << packet.resp_users().resp_value() << endl;
}