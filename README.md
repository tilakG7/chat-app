# chat-app
## 1.0 Description
An application which allows folks using different computers to communicate with
each other.

## 2.0 Communication Model - Client/Server
Each computer is a client which connects to a centralized server. Upon 
connection, client can make requests to communicate with other users.


## 3.0 Requirements
For communication, a client might need to:
1. Notify the network of their presence
2. Know who else is online/available right now (as real time as possible)
3. Send messages to an online person (including when they sent the message)
4. Receive messages from a person (knowing when the message was sent)

Thus, assuming a client/server model where the client initiates all requests 
with the server:
1. Client must register the user with the server
2. Client shall periodically request the list of online users
3. Client shall be able to request to send a message to an online user
4. Client shall periodically request the list of incoming messages

For orchestrating the information exchange, a server would need to:
1. Keep track of online users
2. Identify the type of request and the user sending the request
3. Store incoming msgs from user X intended for user Y
4. Respond to requests for any incoming messages from user Y

## 4.0 API
### 4.1 Header
Each message will have a header consisting of message type and payload length
| Message Type - 8 bits |Payload Length - 32 bits |
|-----------------------|-------------------------|
|0x01 - ?               | 24                      |

| Message Type          |     Value - 8 bits      |
|-----------------------|-------------------------|
| requestRegister       |   0x00                  |
| requestUsers          |   0x01                  |
| requestSend           |   0x02                  |
| requestRecv           |   0x03                  |
|                       |                         |
| respRegister          |   0xF0                  |
| respUsers             |   0xF1                  |
| respSend              |   0xF2                  |
| respRecv              |   0xF3                  |


### 4.2 Request - register user
| Type - 8 bits |Name | Name |Quantity|
|---------------|--------|-----|--------|
|Apple|3       |Apple|3       |
|Egg  |12      |Egg  |12      |

## Additional Features to Consider
1. Broadcasting to multiple users in a group via multicast and UDP
2. Adding timestamping to the messages
3. Use Proto Buffs to handle encoding/decoding of data
4. Use a logging library?