﻿= The protobuf-c Library =

This library is used by the generated code;
it includes common structures and enums,
as well as functions that most users of the generated code
will want.

There are three main components:
  * the Descriptor structures
  * helper structures and objects
  * packing and unpacking code

## The Descriptor Structures ##
For example, enums are described in terms of structures:

```
    struct _ProtobufCEnumValue
    {
      const char *name;
      const char *c_name;
      int value;
    };

    struct _ProtobufCEnumDescriptor
    {
      const char *name;
      const char *short_name;
      const char *package_name;

      /* sorted by value */
      unsigned n_values;
      const ProtobufCEnumValue *values;

      /* sorted by name */
      unsigned n_value_names;
      const ProtobufCEnumValue *values_by_name;
    };
```
Note that n\_values is the number of unique numerical values for the enum, whereas n\_value\_names may include multiple entries for the same value.  In general, use of enums with multiple names for the same value can be troublesome for end-users, so it's best avoided, except perhaps in the case of back-compatibility.

Likewise, messages are described by:
```
    typedef enum
      {
        PROTOBUF_C_LABEL_REQUIRED,
        PROTOBUF_C_LABEL_OPTIONAL,
        PROTOBUF_C_LABEL_REPEATED
      } ProtobufCLabel;

      typedef enum
      {
        PROTOBUF_C_TYPE_INT32,
        PROTOBUF_C_TYPE_SINT32,
        PROTOBUF_C_TYPE_SFIXED32,
        PROTOBUF_C_TYPE_INT64,
        PROTOBUF_C_TYPE_SINT64,
        PROTOBUF_C_TYPE_SFIXED64,
        PROTOBUF_C_TYPE_UINT32,
        PROTOBUF_C_TYPE_FIXED32,
        PROTOBUF_C_TYPE_UINT64,
        PROTOBUF_C_TYPE_FIXED64,
        PROTOBUF_C_TYPE_FLOAT,
        PROTOBUF_C_TYPE_DOUBLE,
        PROTOBUF_C_TYPE_BOOL,
        PROTOBUF_C_TYPE_ENUM,
        PROTOBUF_C_TYPE_STRING,
        PROTOBUF_C_TYPE_BYTES,
        PROTOBUF_C_TYPE_MESSAGE,
      } ProtobufCType;

      struct _ProtobufCFieldDescriptor
      {
        const char *name;
        int id;
        ProtobufCFieldLabel label;
        ProtobufCFieldType type;
        unsigned quantifier_offset;
        unsigned offset;
        const void *descriptor;       /* for MESSAGE and ENUM types */
        const void *default_value;
      };
      struct _ProtobufCMessageDescriptor
      {
        const char *name;
        const char *short_name;
        const char *package_name;

        unsigned n_fields;
        const ProtobufCFieldDescriptor *fields;
      };
```


And finally services are described by:
```
      struct _ProtobufCMethodDescriptor
      {
        const char *name;
        const ProtobufCMessageDescriptor *input;
        const ProtobufCMessageDescriptor *output;
      };
      struct _ProtobufCServiceDescriptor
      {
        const char *name;
        unsigned n_methods;
        ProtobufCMethodDescriptor *methods;             // sorted by name
      };
```

### A Small Optimization: ProtobufCIntRange ###
Rather often, values come in consecutive bunches.  For example, an enum may be defined:
```
      enum HttpStatus
      {
        CONTINUE = 100,
        SWITCHING_PROTOCOLS = 101,
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        ... [36 values follow in http 1.1]
      }
```
To get a name from a value, doing a binary search is rather wasteful.  Instead we make an array of "IntRanges" to optimize the lookup.  There are only five IntRanges required to represent HTTP: each block starting at a multiple of 100:  100, 200, 300, 400, 500.
The IntRange is defined in protobuf-c-private.h as
```
      struct _ProtobufCIntRange
      {
         int start_value;
         unsigned orig_index;
      };
```
The number of elements for each range is inferred from the next value in the array; a dummy value at the end gives the size of the last range.

and for the HTTP example we'd have:
```
     static ProtobufCIntRange private[6] = {
       { 100, 0 },  /* 2 elements */
       { 200, 2 },  /* 7 elements */
       { 300, 9 }   /* 7 elements */
       { 400, 16 }, /* 18 elements */
       { 500, 34 }, /* 6 elements */
       { 0, 40 }
     };      
```
Upshot: lookup is O(log2 n\_ranges) instead of O(log2 n\_values).


## Helper Structures and Typedefs ##

We defined typedefs for a few types
which are used in .proto files but do not
have obvious standard C equivalents:

  * a boolean type (`protobuf_c_boolean`)
  * a binary-data (bytes) type (`ProtobufCBinaryData`)
  * the various int types (`int32_t`, `uint32_t`, `int64_t`, `uint64_t`) are obtained by including `inttypes.h`

We also define a simple allocator object, `ProtobufCAllocator`
that let's you control how allocations are done.
This is predominately used for parsing.

There is a virtual buffer facility that
only has to implement a method to append binary-data
to the buffer.  This can be used to serialize messages
to different targets (instead of a flat slab of data).

We define a base-type for all messages,
for code that handles messages generically.
All it has is the descriptor object.

## Virtual Buffers ##
One important helper type is the `ProtobufCBuffer`
which allows you to abstract the target of serialization.  The only
thing that a buffer has is an `append` method:
```
   struct _ProtobufCBuffer
   {
     void (*append)(ProtobufCBuffer     *buffer,
                    size_t               len,
                    const unsigned char *data);
   }
```
`ProtobufCBuffer` subclasses are often defined on the stack.

For example, to write to a `FILE*` (a file-pointer from `stdio.h`) you could make:
```
   typedef struct
   {
     ProtobufCBuffer base;     /* 'base' must be first member of Buffer subclass */
     FILE *fp;
     protobuf_c_boolean had_error;
   } BufferAppendToFile
   static void my_buffer_file_append (ProtobufCBuffer   *buffer,
                                      unsigned         len,
                                      const unsigned char *data)
   {
     BufferAppendToFile *file_buf = (BufferAppendToFile *) buffer;
     if (!file_buf->had_error)
       {
         if (fwrite (data, len, 1, file_buf->fp) != 1)
           file_buf->had_error = 1;
       }
   }
```

To use this new type of Buffer, you would do something like:
```
     ...
     BufferAppendToFile tmp;
     tmp.base.append = my_buffer_file_append;
     tmp.fp = fp;
     tmp.had_error = 0;
     protobuf_c_message_pack_to_buffer (&message, &tmp);
     if (tmp.had_error)
       die ("error serializing to file");
```

A commonly-used builtin subtype is the `BufferSimple`
which is declared on the stack and uses a scratch buffer provided by the user
for its initial allocation.  It does exponential resizing.
To create a `BufferSimple`, use code like:
```
    unsigned char pad[128];
    ProtobufCBufferSimple buf = PROTOBUF_C_BUFFER_SIMPLE_INIT (pad);
    ProtobufCBuffer *buffer = (ProtobufCBuffer *) &simple;
    protobuf_c_buffer_append (buffer, 6, (unsigned char *) "hi mom");
```
You can access the data as buf.len and buf.data. For example,
```
   assert (buf.len == 6);
   assert (memcmp (buf.data, "hi mom", 6) == 0);
```
To finish up, use:
```
    PROTOBUF_C_BUFFER_SIMPLE_CLEAR (&buf);
```

## packing and unpacking messages ##

To pack messages you first compute their packed size,
then provide a buffer to pack into.
```
    size_t protobuf_c_message_get_packed_size
                                     (ProtobufCMessage *message);
    void   protobuf_c_message_pack   (ProtobufCMessage *message,
                                      unsigned char    *out);
```

Or you can use the "streaming" approach:
```
    void   protobuf_c_message_pack_to_buffer
                                     (ProtobufCMessage *message,
                                      ProtobufCBuffer  *buffer);
```
where `ProtobufCBuffer` is a base object with an append metod.
See the Virtual Buffer section.


To unpack messages, you should simply call
```
      ProtobufCMessage *
         protobuf_c_message_unpack (const ProtobufCMessageDescriptor *,
                                    ProtobufCAllocator  *allocator,
				    size_t               len,
				    const unsigned char *data);
```
If you pass NULL for `allocator`, then
the default allocator will be used.

You can cast the result to the type that matches
the descriptor.

The result of unpacking should be freed with protobuf\_c\_message\_free\_unpacked().