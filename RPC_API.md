# Introduction #
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
We return a `ProtobufC_RPC_Server` object to allow for additional configuration
and monitoring of the server.


# Constructing a Client #
```
typedef enum
{
  PROTOBUF_C_RPC_ADDRESS_LOCAL,  /* unix-domain socket */
  PROTOBUF_C_RPC_ADDRESS_TCP     /* host/port tcp socket */
} ProtobufC_RPC_AddressType;

ProtobufCService *protobuf_c_rpc_client_new (ProtobufC_RPC_AddressType type,
                                             const char               *name,
                                             const ProtobufCServiceDescriptor *descriptor,
                                             ProtobufCDispatch       *dispatch);
```
Create a new connection to a protobuf-c-rpc server.  _type_ and _name_
give the location of the server.
  * if _type_ is `PROTOBUF_C_RPC_ADDRESS_LOCAL` then the _name_ is a path in the local filesystem
  * if _type_ is `PROTOBUF_C_RPC_ADDRESS_TCP` then the _name_ is a host:port combination.

The _descriptor_ gives the service format data (e.g. which methods exist and their types).

The _dispatch_ is the main-loop with which
to register file-descriptors and timeouts.  If given `NULL` if
will use the default dispatch.

The client does not block until it is connected.
Sometimes you want to do just that:
```
typedef enum
{
  PROTOBUF_C_RPC_CLIENT_CONNECT_SUCCESS,
  PROTOBUF_C_RPC_CLIENT_CONNECT_ERROR_NAME_LOOKUP,
  PROTOBUF_C_RPC_CLIENT_CONNECT_ERROR_CONNECT
} ProtobufC_RPC_Client_ConnectStatus;
ProtobufC_RPC_Client_ConnectStatus
protobuf_c_rpc_client_connect (ProtobufC_RPC_Client *client);
```

# Configuring the Client #
```
void protobuf_c_rpc_client_set_name_resolver (ProtobufC_RPC_Client *client,
                                              ProtobufC_NameLookup_Func resolver);
```
This is an API to allow you to plug in a different name-resolver.  In particular an asynchronous DNS
implementation would be really nice.  Anybody want to contribute one that I can
put in the distro?

```
typedef void (*ProtobufC_RPC_Error_Func)   (ProtobufC_RPC_Error_Code code,
                                            const char              *message,
                                            void                    *error_func_data);

void protobuf_c_rpc_client_set_error_handler (ProtobufC_RPC_Client *client,
                                              ProtobufC_RPC_Error_Func func,
                                              void                 *error_func_data);
```
Set a function that will be called whenever an error occurs with the client.

```
void protobuf_c_rpc_client_disable_autoretry (ProtobufC_RPC_Client *client);
void protobuf_c_rpc_client_set_autoretry_period (ProtobufC_RPC_Client *client,
                                                 unsigned              millis);
```
These functions can be used to configure whether the client
will automatically try to connect again if it is disconnected.  The default is to try
reconnecting rather frequently.

```
protobuf_c_boolean protobuf_c_rpc_client_is_connected (ProtobufC_RPC_Client *client);
```
Return `TRUE` if the client is connected to the server.  If the client is not connected,
all requests to the client will fail immediately.

# API Details: Constructing a Server #

```
ProtobufC_RPC_Server *
     protobuf_c_rpc_server_new        (ProtobufC_RPC_AddressType type,
                                       const char               *name,
                                       ProtobufCService         *service,
                                       ProtobufCDispatch       *dispatch);
```

# API Details:  Configuring the Server #
```
void protobuf_c_rpc_server_disable_autotimeout(ProtobufC_RPC_Server *server);
void protobuf_c_rpc_server_set_autotimeout (ProtobufC_RPC_Server *server,
                                            unsigned              timeout_millis);
```
unimplemented.  disconnect a client that has been idle too long.

```
void protobuf_c_rpc_server_set_error_handler (ProtobufC_RPC_Server *server,
                                              ProtobufC_RPC_Error_Func func,
                                              void                 *error_func_data);
```
Register a function to call if an error occurs.

# Cleaning up #
```
void protobuf_c_service_destroy       (ProtobufCService         *client);
ProtobufCService *
     protobuf_c_rpc_server_destroy    (ProtobufC_RPC_Server     *server,
                                       protobuf_c_boolean        free_underlying_service);
```

# ProtobufCDispatch #
There are two distinct users of this code:
  * users of the main-loop: those who wish to register watches on file-descriptors, timeouts and idle functions.
  * those embedding the dispatch into an existing application's main-loop: those who have an existing main-loop or other infrastructure that must now run a protobuf-c rpc server or client.  See [RPC\_Embedding](RPC_Embedding.md) where we hope to provide useful examples.

# Dispatch User API #
Here we describe the functions to handle file-descriptor events, timeouts and idle functions.

```
 ... suck in from protobuf-c-dispatch.h
```

## Dispatch Embedding API ##
Here we describe succinctly the functions that will be used by developers embedding a `ProtobufCDispatch` into an existing main-loop infrastructure.