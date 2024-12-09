
// TODO: Implement better control logic
#include <iostream>

#include "server_state.h"

using namespace std;
using length_t = uint32_t;
enum class PacketType : uint8_t {
    kRequestRegister = 0,
    kRequestUsers = 1,
    kRequestSend = 2,
    kRequestRecv = 3,
    kRespRegister = 0xF0,
    kRespUsers = 0xF1,
    kRespSend = 0xF2,
    kRespRecv = 0xF3
};

struct __attribute__((packed)) Header {
    PacketType type;
    length_t len;
};

constexpr size_t kMinPayloadLen = 1; // for server side

class MCCServer {
public:
    MCCServer(ServerState& storage) : storage_(storage){}

    /**
     * Parses the register request
     * @param data        - pointer to payload of register request
     * @param payload_len - length of payload
     */
    void parseRequestRegister(uint8_t *data, length_t payload_len);

    /**
     * Parses the get online users request
     * @param data         - pointer to payload of request
     * @param payload_len   
     */
    void MCCServer::parseRequestUsers(uint8_t *data, length_t payload_len);

    /**
     * Parses 1 request from the client and sends a response back
     * @param data - pointer to data buffer
     * @param size - number of bytes in buffer pointed to by data
     */
    void parse(uint8_t *data, size_t size);
private:
    ServerState& storage_;   // reference to the storage instance
    vector<uint8_t>& tx_buffer_; // buffer of data to transmit to client
};