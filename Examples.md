# Introduction #

Examples of using [protobuf-c](http://www.google.com/codesearch/p?hl=en#Cn85FSbHSs0/trunk/Dependencies/protobuf-2.0.3/src/protobuf-c.c&q=protobuf-c&d=4).

protobuf-c works by taking a .proto file (which is defined by google's [Protocol Buffers](http://code.google.com/apis/protocolbuffers/) library), and generating both .h and .c files for use in C programs.

**Warning (insert guy shoveling pic):  this page is partial, incomplete and the code is untested.  Eventually I plan to make an "examples" directory in the .tar.gz so you don't need to cut-n-paste.**

# Simple complete example #


A simple file, `amessage.proto` file following the [language guide](http://code.google.com/apis/protocolbuffers/docs/proto.html):
```
   message AMessage
   {
      required int32 a=1; 
      optional int32 b=2;
   }
```

Generate .h and .c files from the command-line in your current working directory:
```
   protoc-c --c_out=. amessage.proto 
```

Serialize/pack the AMessage as follows:
```
#include <stdio.h>
#include <stdlib.h>
#include "amessage.pb-c.h"

int main (int argc, const char * argv[]) 
{
    AMessage msg = AMESSAGE__INIT; // AMessage
    void *buf;                     // Buffer to store serialized data
    unsigned len;                  // Length of serialized data
	
    if (argc != 2 && argc != 3)
    {   // Allow one or two integers
        fprintf(stderr,"usage: amessage a [b]\n");
        return 1;
    }
	
    msg.a = atoi(argv[1]);
    if (argc == 3) { msg.has_b = 1; msg.b = atoi(argv[2]); }
    len = amessage__get_packed_size(&msg);
	
    buf = malloc(len);
    amessage__pack(&msg,buf);
	
    fprintf(stderr,"Writing %d serialized bytes\n",len); // See the length of message
    fwrite(buf,len,1,stdout); // Write to stdout to allow direct command line piping
	
    free(buf); // Free the allocated serialized buffer
    return 0;
}
```
I left most error handling out for brevity.  Notice:
  * the use of the `A_MESSAGE__INIT` macro to construct the message
  * the `has_b` member corresponds to the optional `b` field -- required fields do not have a `has_` member.
  * `amessage__get_packed_size` returns the length of the packed data.
  * `a_message__pack` serializes the message.

On the other hand, to deserialize/unpack a message, try code like this:
```
#include <stdio.h>
#include <stdlib.h>
#include "amessage.pb-c.h"
#define MAX_MSG_SIZE 1024

static size_t
read_buffer (unsigned max_length, uint8_t *out)
{
  size_t cur_len = 0;
  uint8_t c;
  while ((nread=fread(out + cur_len, 1, max_length - cur_len, stdin)) != 0)
    {
      cur_len += nread;
      if (cur_len == max_length)
        {
          fprintf(stderr, "max message length exceeded\n");
          exit(1);
        }
    }
  return cur_len;
}


int main (int argc, const char * argv[]) 
{
  AMessage *msg;

  // Read packed message from standard-input.
  uint8_t buf[MAX_MSG_SIZE];
  size_t msg_len = read_buffer (MAX_MSG_SIZE, buf);

  // Unpack the message using protobuf-c.
  msg = amessage__unpack(NULL, msg_len, buf);	
  if (msg == NULL)
    {
      fprintf(stderr, "error unpacking incoming message\n");
      exit(1);
    }

  // display the message's fields.
  printf("Received: a=%d",msg->a);  // required field
  if (msg->has_b)                   // handle optional field
    printf("  b=%d",msg->b);
  printf("\n");

  // Free the unpacked message
  amessage__free_unpacked(msg, NULL);
  return 0;
}
```
During linking each above program, make sure to include '-lprotobuf-c'

Test by piping one program into the next at command line:
```
./amessage_serialize 10 2 | ./amessage_deserialize 
Writing: 4 serialized bytes
Received: a=10 b=2
```

# Repeated Fields #
Here is a simple file, `cmessage.proto` file:
```
message CMessage
{
    repeated int32 c=1;
}
```

Serialize/pack the CMessage as follows:
```
#include <stdio.h>
#include <stdlib.h>
#include "cmessage.pb-c.h"

int main (int argc, const char * argv[]) 
{
    CMessage msg = CMESSAGE__INIT;  // CMessage (repeated int32)
    void *buf;                      // Buffer to store serialized data
    unsigned len,i;                 // Length of serialized data
    
    msg.n_c = argc - 1;                      // Save number of repeated int32
    msg.c = malloc (sizeof (int) * msg.n_c); // Allocate memory to store int32
    for (i = 0; i < msg.n_c; i++)
        msg.c[i] = atoi (argv[i+1]);         // Access msg.c[] as array 
    len = cmessage__get_packed_size (&msg);  // This is calculated packing length
    buf = malloc (len);                      // Allocate required serialized buffer length 
    cmessage__pack (&msg, buf);              // Pack the data

    fprintf(stderr,"Writing %d serialized bytes\n",len); // See the length of message
    fwrite (buf, len, 1, stdout);            // Write to stdout to allow direct command line piping

    free (msg.c); // Free storage for repeated int32             
    free (buf);   // Free serialized buffer
    return 0;
}
```
I left most error handling out for brevity.  Notice:
  * the use of the `C_MESSAGE__INIT` macro to construct the message
  * the n\_XXX member is generated for a repeated field XXX.

On the other hand, if you want to deserialize/unpack a message, try code like this:
```
#include <stdio.h>
#include "cmessage.pb-c.h"
#define MAX_MSG_SIZE  4096

int main (int argc, const char * argv[]) 
{
    CMessage *msg;
    uint8_t buf[MAX_MSG_SIZE];
    size_t msg_len = read_buffer (MAX_MSG_SIZE, buf);

    msg = cmessage__unpack (NULL, len, buf); // Deserialize the serialized input
    if (msg == NULL)
    { // Something failed
        fprintf(stderr, "error unpacking incoming message\n");
        return 1;
    }
    
    for (i = 0; i < msg->n_c; i++)
    { // Iterate through all repeated int32
        if (i > 0)
            printf (", ");
        printf ("%d", msg->c[i]);
    }
    printf ("\n");
    
    cmessage__free_unpacked(msg,NULL); // Free the message from unpack()
    return 0;
}
```
Test by piping one program into the next at command line:
```
./cmessage_serialize 12 3 4 | ./cmessage_deserialize 
Writing: 6 serialized bytes
12, 3, 4
```
# Strings #
TODO
# Bytes #
TODO
# Constructing Submessages #
Here is a simple file, `dmessage.proto` file:
```
message Submessage
{
    required int32 value=1;
}

message DMessage
{
    required Submessage a=1;
    optional Submessage b=2;
}
```

Here `DMessage` consists of one or two integers (`a` is one int and required; `b` is one int and optional).

```
#include <stdio.h>
#include <stdlib.h>
#include "dmessage.pb-c.h"

int main (int argc, const char * argv[]) 
{
    DMessage msg    = DMESSAGE__INIT;   // DMESSAGE
    Submessage sub1 = SUBMESSAGE__INIT; // SUBMESSAGE A
    Submessage sub2 = SUBMESSAGE__INIT; // SUBMESSAGE B
    void *buf;
    unsigned len;
    
    if (argc != 2 && argc != 3)
    {   // Allow one or two integers
        fprintf(stderr,"usage: dmessage a [b]\n");
        return 1;
    }
    sub1.value = atoi (argv[1]); 
    msg.a = &sub1;               // Point msg.a to sub1 data
    
    // NOTE: has_b is not required like amessage, therefore check for NULL on deserialze
    if (argc == 3) { sub2.value = atoi (argv[2]); msg.b = &sub2; } // Point msg.b to sub2 data
    
    len = dmessage__get_packed_size (&msg); // This is the calculated packing length
    buf = malloc (len);                     // Allocate memory
    dmessage__pack (&msg, buf);             // Pack msg, including submessages

    fprintf(stderr,"Writing %d serialized bytes\n",len); // See the length of message
    fwrite (buf, len, 1, stdout);           // Write to stdout to allow direct command line piping
    
    free(buf); // Free the allocated serialized buffer
    return 0;
}
```
Notice:
  * there is no `has_` flag for optional submessages -- if the pointer is non-NULL, then we assume that it is a value.

On the other hand, if you want to deserialize/unpack a message, try code like this:
```
#include <stdio.h>
#include "dmessage.pb-c.h"
#define MAX_MSG_SIZE 4096

int main (int argc, const char * argv[]) 
{
    DMessage *msg;         // DMessage using submessages
    Submessage *sub1,*sub2;// Submessages
    char c; int i=0;       // Data holders
    uint8_t buf[MAX_MSG_SIZE]; // Input data container for bytes
    
    while (fread(&c,1,1,stdin) != 0)
    {
        if (i >= MAX_MSG_SIZE)
        {
            fprintf(stderr,"message too long for allocated buffer\n");
            return 1;
        }
        buf[i++] = c;
    }
    
    msg = dmessage__unpack(NULL,i,buf); // Deserialize the serialized input
    if (msg == NULL)
    { // Something failed
        fprintf(stderr,"error unpacking incoming message\n");
        return 1;
    }
    sub1 = msg->a; sub2 = msg->b;
    printf("Received: a=%d",sub1->value);
    if (msg->b != NULL) printf(" b=%d",sub2->value);
    printf("\n");
    
    dmessage__free_unpacked(msg,NULL);
    
    return 0;
}
```
Test by piping one program into the next at command line:
```
./dmessage_serialize 4 5 | ./dmessage_deserialize 
Writing: 8 serialized bytes
Received: a=4 b=5
./dmessage_serialize 4 | ./dmessage_deserialize 
Writing: 4 serialized bytes
Received: a=4
```
# Constructing Repeated Submessages #
Here is a simple file, `emessage.proto` file:
```
message Submessage
{
    required int32 value=1;
}

message EMessage
{
    repeated Submessage a=1;
}
```

Here `DMessage` consists of one or two integers (`a` is one int and required; `b` is one int and optional).

```
#include <stdio.h>
#include <stdlib.h>
#include "dmessage.pb-c.h"

int main (int argc, const char * argv[]) 
{
    Submessage **subs;
    void *buf;
    unsigned len;
    subs = malloc (sizeof (Submessage*) * (argc-1));
    for (i = 1; i < argc; i++)
       {
         subs[i-1] = malloc (sizeof (Submessage));
         submessage__init (subs[i-1]);
         subs[i-1]->value = atoi(argv[i]);
      }    
    msg.n_a = argc-1;
    msg.a = subs;
    len = emessage__get_packed_size (&msg); // This is the calculated packing length
    buf = malloc (len);                     // Allocate memory
    emessage__pack (&msg, buf);             // Pack msg, including submessages

    fprintf(stderr,"Writing %d serialized bytes\n",len); // See the length of message
    fwrite (buf, len, 1, stdout);           // Write to stdout to allow direct command line piping
    
    free(buf); // Free the allocated serialized buffer
   for (i = 1; i < argc; i++)
      free (subs[i]);
   free (subs); 
   return 0;
}
```
Notice that repeated fields create two fields, in this case `n_a`, the number of submessages, and `a` the submessages themselves.  Also note that `a` is an array of _pointers_ to messages.

On the other hand, if you want to deserialize/unpack a message, try code like this:
```
#include <stdio.h>
#include "emessage.pb-c.h"
#define MAX_MSG_SIZE 4096

int main (int argc, const char * argv[]) 
{
    EMessage *msg;
    char c; int i=0;
    uint8_t buf[MAX_MSG_SIZE]; // Input data container for bytes
    
    while (fread(&c,1,1,stdin) != 0)
    {
        if (i >= MAX_MSG_SIZE)
        {
            fprintf(stderr,"message too long for allocated buffer\n");
            return 1;
        }
        buf[i++] = c;
    }
    
    msg = emessage__unpack(NULL,i,buf); // Deserialize the serialized input
    if (msg == NULL)
    {
        fprintf(stderr,"error unpacking incoming message\n");
        return 1;
    }
    for (i = 0; i < msg->n_a; i++)
       printf ("%d\n", msg->a[i]->value);
    
    emessage__free_unpacked(msg,NULL);
    
    return 0;
}
```
Test by piping one program into the next at command line:
```
./emessage_serialize 4 5 | ./emessage_deserialize 
Writing: 8 serialized bytes
4
5
./emessage_serialize 4 5 8| ./emessage_deserialize 
4
5
8
```

# Using the Allocator #

# Packing with an Append Function #

# Language Binding Hints #

# Protobuf-C For Embedded Developers (and other cross compilers) #
TODO: move these general comments to a new page and provide an example, as promised by the page title.

Most embedded developers need to compile protobuf-c twice:
  1. they need a copy of `protoc-c` that runs on their `build` environment (the `build` environment is for the machine on which they develop the application.
  1. they need to have a copy of libprotobuf-c, which is the runtime library, that is compiled for the embedded platform (which we will call the `target` environment).

For (1), you need to install the main protobuf package, since protoc-c uses its parser and code generator libraries.  Running 'configure' as usual should be fine.

For (2), you can configure with --disable-protoc which means you won't have to build the main protobuf package for the embedded platform (since you don't need to do code gen on the embedded platform).  The usual details of cross-compilation for your specific embedded platform apply.  Sometimes merely providing the name of the platform to configure suffices (as in, `./configure atmel`)-- but other environments need a mass of overrides like `CC=atmel-cc LD=atmel-ld`.

If embedded developers do not modify the `.proto` files, then then all they really need are the generated .pb-c.[ch](ch.md) files, and then they can skip (1).