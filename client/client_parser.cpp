
#include "client_parser.h"
#include "common/packet.pb.h"

#include <sstream>
#include <string>

using namespace std;

// needs to write 
string ClientParser::encodeRequestRegister(const string& username) {
    ostringstream oss;
    Header hdr;
    hdr.set_packet_type(Header::PACKET_TYPE_REQUEST_REGISTER);
    if(!hdr.SerializeToOstream(&oss)) {
        cerr << "Error parsing Header to ostream" << endl;
    }
    RequestRegister req;
    req.set_username(username);
    req.SerializeToString();
    if(!req.SerializeToOstream(&oss)) {
        cerr << "Error parsing RequestRegister Payload to ostream" << endl;
    }
    
    return oss.str();
}

