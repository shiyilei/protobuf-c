﻿#summary The RPC system provided by protobuf-c
# Introduction #

Protocol buffers provides a mechanism for defining a "service", which is intended to form the basis for an "remote-procedure call" (RPC) system.  They do not actually provide any implementation or protocol suggestions for doing so, both in the name of generality, and because google's internal protocols are tied to their architecture too strongly [cite?].

protobuf-c, by constrast _does_ provide an implementation.  We will first describe the protocol, and then give an API for using it.

# Components of the protobuf-c RPC Implementation #
Remember that a **service** in protocol-buffers are an array of named methods, each of which takes a single message input and provides a single message output; the input and output types are specified in the .proto file.

Our RPC system adds some more ideas:
  * the **client** is the active end of the tcp or unix-domain socket connection.  Once connected, it can invoke functions on the server.  The client object we provide implements automatic reconnection if the server is down.
  * the **server** is the passive end of the tcp or unix-domain connection.  It has an underlying ProtobufCService that it will call at the behest of the clients.  As usual, many clients may be connected to a single server.
  * the **dispatch** is a way of handling multiple connections from a single thread.  We use it for the client and the server.  The idea is the same as a "main-loop": we accept requests for which file-descriptors to watch for which events.  We chose to call it a "dispatch" because it is designed to be embedded in the main loops of applications, although it can also run standalone.

# The Encapsulation Protocol #
We here describe the way to pass messages across the socket; the translation of the messages themselves into bytes is done by core protobuf, but it leaves several unspecified parts:
  * how should the client specify which method to invoke?
  * how should the client and server specify the protobuf-encoded message-length? (It cannot be determined by examining the bytes.)
  * how should failures be passed back to the client?
  * if the protocol allows pipelining, how should we support out-of-order responses? (i.e. how does the client match the server's response to its request?)

Our encapsulation protocol is designed to facilitate pipelining of requests.  It can be much more efficient to pipeline many small requests.

The client issues a 12 byte header followed by the protobuf-encoded message payload:
  * method index (encoded as a 4-byte little-endian number)
  * message length: the length of the protobuf-encoded payload (encoded 4-byte little-endian)
  * request id: a value chosen by the client to allow it to know which server response corresponds to which request, in the case of multiple outstanding requests). (encoded in 4 bytes)

The server eventually issues a similar 16 byte response:
  * status code (as a 4-byte little-endian number).  One of the following values:
    * 0: success
    * 1: service failed (ie passed in NULL for the message to the closure)
    * 2: too many pending (client connection has too many pending requests)
  * method index (same as for request)
  * message length (same as for request)
  * request id (same as for request)

# Programmer's Reference (API Documentation) #

Please see the [RPC\_API](RPC_API.md) page for the complete story.  However, if you understand the rest of the protobuf-c api then this description should be adequate for a cursory use.
See the [RPC\_Example](RPC_Example.md) for complete client and server example code.

The basic gist is:
```
   Service *create_client (Location, ServiceDescriptor)
   void     create_server (Port, Service);
```
So you see:
  * the client connects to the Location and using the message formats given by the ServiceDescriptor.
  * the server binds to the Port and answers requests using the Service.

Of course, as usual, C naming conventions are never that short.  Here's the
real functions:

```
ProtobufCService     *protobuf_c_rpc_client_new (ProtobufC_RPC_AddressType  type,
                                                 const char                *name,
                                                 const ProtobufCServiceDescriptor *descriptor,
                                                 ProtobufCDispatch         *dispatch);
ProtobufC_RPC_Server *protobuf_c_rpc_server_new (ProtobufC_RPC_AddressType  type,
                                                 const char                *name,
                                                 ProtobufCService          *service,
                                                 ProtobufCDispatch         *dispatch);
```
We return a ProtobufC\_RPC\_Server object to allow for additional configuration
and monitoring of the server.

# Implementing a Service #
On the RPC server side closure\_data is used by protobuf-c's RPC
implementation internally (e.g. for keeping track of request id's) and is opaque to the application code that implements the actual underlying service:  it must pass it back it to the protobuf-c generated service closure function that will eventually send the result message
to the client.

# RPC Example #
Sometimes an [example](RPC_Example.md) is much easier to follow than a dry description.

# Threading #
Limited multithreading support is available (as of Mar 22 2011 only from revision-control).  See [RPC Threading for details](RPC_Threading.md).

# TODO #
  * implement protobuf\_c\_rpc\_client\_connect()
  * more exhaustive test code