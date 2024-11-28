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

### 3.1 Mapping requests to users
Since this communication model requires the client making multiple requests,
there needs to be a mechanism of keeping track of a user between requests.


Upon the first request, the client can set their username. To track subsequent 
requests, we can assign the user a unique identifier. The client can then use 
this unique ID to sign each message, identifying the user. 

There are other ways to acheive this as well, such as saving the users IP 
address, but IPs do not always uniquely identify a user.

## 4.0 API
Each packet of data consists of a header and payload.
### 4.1 Header
Header consists of:
1. 1 byte for packet type (see table below)
2. 4 bytes for payload length



| Packet Type          |     Value - 1 byte      |
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


### 4.2 Requests - sent by client
#### 4.2.1 Register (0x00) - request to register a user
Payload holds the username to be registered. Server responds with a 
unique ID identifying the user.
Example:
<table><thead>
  <tr>
    <th colspan="2">Header</th>
    <th>Payload</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Type (1 byte)</td>
    <td>Length (4 bytes)</td>
    <td>Username</td>
  </tr>
  <tr>
    <td>0x00</td>
    <td>6</td>
    <td>Evelyn</td>
  </tr>
</tbody>
</table>

#### 4.2.2 Users (0x01) - request to get oneline users
Payload holds the user ID. Example:

<table><thead>
  <tr>
    <th colspan="2">Header</th>
    <th>Payload</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Type (1 byte)</td>
    <td>Length (4 bytes)</td>
    <td>User ID (4 bytes)</td>
  </tr>
  <tr>
    <td>0x01</td>
    <td>4</td>
    <td>91293</td>
  </tr>
</tbody>
</table>

#### 4.2.3 Send (0x02) - request to send a message
The payload holds user ID of the sender and receiver. The message follows.
<table><thead>
  <tr>
    <th colspan="2">Header</th>
    <th colspan="3">Payload</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Type (1 byte)</td>
    <td>Length (4 bytes)</td>
    <td>Source ID (4 bytes)</td>
    <td>Target ID (4 bytes)</td>
    <td>Message (variable size)</td>
  </tr>
  <tr>
    <td>0x02</td>
    <td>29</td>
    <td>91293</td>
    <td>91297</td>
    <td>Hi Jack, how are you?</td>
  </tr>
</tbody>
</table>

#### 4.2.4 Recv (0x03) - request to receive messages
Payload holds the user ID
<table><thead>
  <tr>
    <th colspan="2">Header</th>
    <th>Payload</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Type (1 byte)</td>
    <td>Length (4 bytes)</td>
    <td>User ID (4 bytes)</td>
  </tr>
  <tr>
    <td>0x03</td>
    <td>4</td>
    <td>91293</td>
  </tr>
</tbody>
</table>


### 4.3 Responses - sent by server
#### 4.3.1 Register (0xF0) - response to request for registering a user
Positive response: server responds w/ a 0 value and a 4 byte unique ID for the 
client to use in subsequent requests.
Negative response: server reponds w/ ONLY a non-zero vaule.


<table><thead>
  <tr>
    <th>Response Type</th>
    <th>Value</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Sucess</td>
    <td>0</td>
  </tr>
  <tr>
    <td>User already exists</td>
    <td>1</td>
  </tr>
</tbody>
</table>

<table><thead>
  <tr>
    <th colspan="3">Always present</th>
    <th colspan="1">Only present on success</th>
  </tr>
  <tr>
    <th colspan="2">Header</th>
    <th colspan="2">Payload</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Type (1 byte)</td>
    <td>Length (4 bytes)</td>
    <td>Response value (1 byte)</td>
    <td>Unique ID (4 bytes)</td>
  </tr>
  <tr>
    <td>0xF0</td>
    <td>1 or 5</td>
    <td>0</td>
    <td>91293</td>
  </tr>
</tbody>
</table>

#### 4.3.2 Users (0xF1) - response to request for getting online users
<table><thead>
  <tr>
    <th>Response Type</th>
    <th>Value</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Sucess</td>
    <td>0</td>
  </tr>
  <tr>
    <td>Invalid User ID Provided</td>
    <td>1</td>
  </tr>
</tbody>
</table>

<table><thead>
  </tr>
    <tr>
    <th colspan="3">Always present</th>
    <th colspan="4">Only present on success</th>
  </tr>
  <tr>
    <th colspan="2">Header</th>
    <th colspan="2">Payload</th>
    <th colspan="3">Repeat for n users</th>
</thead>

<tbody>
  <tr>
    <td>Type (1 byte)</td>
    <td>Length (4 bytes)</td>
    <td>Response value (1 byte)</td>
    <td># users (4 bytes)</td>
    <td>ith user ID (4 bytes)</td>
    <td>len(username) (1 byte)</td>
    <td>username</td>
  </tr>
  <tr>
    <td>0xF1</td>
    <td>>= 1</td>
    <td>0</td>
    <td>1</td>
    <td>91297</td>
    <td>4</td>
    <td>jack</td>
  </tr>
</tbody>
</table>

#### 4.3.3 Send (0xF2) - response to sending data to a user
<table><thead>
  <tr>
    <th>Response Type</th>
    <th>Value</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Sucess</td>
    <td>0</td>
  </tr>
  <tr>
    <td>Invalid source user ID</td>
    <td>1</td>
  </tr>
  <tr>
    <td>Invalid target user ID</td>
    <td>2</td>
  </tr>
</tbody>
</table>


<table><thead>
  <tr>
    <th colspan="2">Header</th>
    <th colspan="2">Payload</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Type (1 byte)</td>
    <td>Length (4 bytes)</td>
    <td>Response value (1 byte)</td>
  </tr>
  <tr>
    <td>0xF2</td>
    <td>1</td>
    <td>0</td>
  </tr>
</tbody>
</table>

#### 4.3.4 Recv (0xF3) - response to request for incoming messages
<table><thead>
  <tr>
    <th>Response Type</th>
    <th>Value</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Sucess</td>
    <td>0</td>
  </tr>
  <tr>
    <td>Invalid User ID Provided</td>
    <td>1</td>
  </tr>
</tbody>
</table>

<table><thead>
  </tr>
    <tr>
    <th colspan="3">Always present</th>
    <th colspan="4">Only present on success</th>
  </tr>
  <tr>
    <th colspan="2">Header</th>
    <th colspan="1">Payload</th>
    <th colspan="4">Repeat for n users</th>
  </tr></thead>
<tbody>
  <tr>
    <td>Type (1 byte)</td>
    <td>Length (4 bytes)</td>
    <td>Response value (1 byte)</td>
    <td># messages (4 bytes)</td>
    <td>ith user ID (4 bytes)</td>
    <td>len(message) (1 byte)</td>
    <td>message</td>
  </tr>
  <tr>
    <td>0xF3</td>
    <td>>= 1</td>
    <td>0</td>
    <td>1</td>
    <td>91297</td>
    <td>21</td>
    <td>I'm well Evelyn, you?</td>
  </tr>
</tbody>
</table>

## Additional Features to Consider
1. Broadcasting to multiple users in a group via multicast and UDP
2. Adding timestamping to the messages
3. Use Proto Buffs to handle encoding/decoding of data
4. Use a logging library?