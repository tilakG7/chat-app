
#include "client_parser.h"
#include "common/packet.pb.h"

#include <sstream>
#include <string>

using namespace std;

// needs to write 
string ClientParser::encodeRequestRegister(const string& username) {
    ostringstream oss;
    mcc::Header hdr;
    hdr.set_packet_type(mcc::Header::PACKET_TYPE_REQUEST_REGISTER);
    if(!hdr.SerializeToOstream(&oss)) {
        cerr << "Error parsing Header to ostream" << endl;
    }
    mcc::RequestRegister req;
    req.set_username(username);
    cout << "Setting username: " << username << endl;
    if(!req.SerializeToOstream(&oss)) {
        cerr << "Error parsing RequestRegister Payload to ostream" << endl;
    }
    return oss.str();
}

