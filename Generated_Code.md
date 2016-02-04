﻿= The Generated Code =
For each enum, we generate a C enum.
For each message, we generate a C structure
which can be cast to a `ProtobufCMessage`.

For each enum and message, we generate a descriptor
object that allows us to implement a kind of reflection
on the structures.

# Naming and Casing Conventions #

  * The name of the type for enums and messages and services is camel case (meaning WordsAreCrammedTogether) except that double-underscores are used to delimit scopes.  For example:
```
                package foo.bar;
                message BazBah {
                  required int32 val = 1;
                  optional string val2 = 2;
                }
```
> would generate a C type `Foo__Bar__BazBah`.

  * Functions and globals are all lowercase, with camel-case words separated by single underscores.  For example:
```
           Foo__Bar__BazBah *foo__bar__baz_bah__unpack
                                 (ProtobufCAllocator  *allocator,
                                  size_t length,
                                  const uint8_t *data);
```
  * Enums values are all uppercase.
  * Stuff we add to your symbol names will also be separated by a double-underscore.  For example, the unpack method above.

# Descriptors #

We also generate descriptor objects for messages and enums.
These are declared in the .h files:
```
   extern const ProtobufCMessageDescriptor
                     foo__bar__baz_bah__descriptor;
```

For the definitions of the various `Descriptor` structures,
see the [libprotobuf\_c](libprotobuf_c.md) page.

# Message Methods #

To initialize a message for yourself,
use the uppercased `__INIT` macro:
For example:
```
    Foo__Bar__BazBah message = FOO__BAR__BAZ_BAH__INIT;
```
This will properly handle [default values](Default_Values.md).

Sometimes, a function that initializes a message of a give type is useful.
For that purpose, we provide an `__init()` function
```
    Foo__Bar__BazBah message;
    foo__bar__baz_bah__init (&message);
```

We generate some functions for each message:
  * `init()`.  Equivalent to assigning the `__INIT` macro to its argument.  It's useful in some contexts, like initializing a block of memory allocated by the user.
  * `unpack()`.  Unpack data for a particular message-format, using an optional allocator for the data:
```
     Foo__Bar__BazBah *
     foo__bar__baz_bah__unpack  (ProtobufCAllocator *allocator,
                                 size_t length,
                                 const unsigned char *data);
```
> > It's a good idea to not modify the unpacked message.  That's because we haven't decided exactly whether the message's fields are allocated separately or together.  The only thing you can really assume is that free\_unpacked() will always to the right thing with the return value from unpack().
> > In practice, we will never be allocating memory for required and optional primitive types, so you CAN modify those values before calling free\_unpacked().


> To make other kinds of changes, make a copy and track your own allocations:
```
    Foo__Bar__BazBah *unpacked;
    Foo__Bar__BazBah my_bazbah;
    unpacked = foo__bar__baz_bah__unpack (NULL, len, data);
    my_bazbah = *unpacked;
    my_bazbah.val2 = "my string";
    ... when done with unpacked and my_bazbah, call free_unpacked()
```
  * `free_unpacked()`.  Free a message that you obtained with the unpack method:
```
     void
     foo__bar__baz_bah__free_unpacked  (Foo__Bar__BazBah *baz_bah,
                                        ProtobufCAllocator *allocator);
```
  * `get_packed_size()`.  Find how long the serialized representation of the data will be:
```
     size_t
     foo__bar__baz_bah__get_packed_size 
                        (const Foo__Bar__BazBah *message);
```
  * `pack()`.  Pack message into buffer; assumes that buffer is long enough (use get\_packed\_size first!).
```
     size_t
     foo__bar__baz_bah__pack
                        (const Foo__Bar__BazBah *message,
                         unsigned char *packed_data_out);
```
  * `pack_to_buffer()`.  Pack message into virtualized buffer.
```
     size_t
     foo__bar__baz_bah__pack_to_buffer
                        (const Foo__Bar__BazBah *message,
                         ProtobufCBuffer *buffer);
```

For the definitions of `ProtobufCBuffer` and `ProtobufCAllocator`,
see the [libprotobuf\_c](libprotobuf_c.md) page.

Finally, note that [default-value handling](Default_Values.md) is a little subtle- it'll probably mostly work as you expect (it's mostly modeled after the c++ implementation), but it's got a quirk regarding optional string values.  There is no "has\_field" for a string field.  To determine if a string-field is set to the default value, use something like:
```
#define my_struct_has_field(my_struct) \
   (my_struct.field != NULL            \
 && my_struct.field != my_struct__field__default_value)
```
(see the "default-value handling" link for the details).
## Services ##
Services are collections of methods each having an input and output type.
Unlike messages where we generate a structure that corresponds
to the actual message object, for services we generate
a function that creates a `ProtobufCService`
from a collection of user-defined methods.

We also define simple functions that invoke each method of a service.
These functions work if the service is created by
the `create_service` generated function
or if the service is instantiated by an RPC system.

Suppose we have a .proto file:
```
     message A {
       required uint32 val = 1;
     }
     message B {
       required string foo = 1;
     }
     service Convert {
       rpc Itoa (A) returns (B);
       rpc Atoi (B) returns (A);
     }
```

We will get generated code:
```
    struct _Convert_Service {
      ProtobufCService base;

      void (*itoa) (Convert_Service *service,
                    const A *input,
                    B__Closure closure,
                    void *closure_data);
      void (*atoi) (Convert_Service *service,
                    const B *input,
                    A__Closure closure,
                    void *closure_data);
      void (*destroy) (Convert_Service *service);
    };
```

Note that `create_service` takes ownership of `service`.
For example, here's how to implement a convert service that takes the default radix to use:
(XXX: untested)
```
      /* structure derived from Convert_Service. */
      typedef struct {
        Convert_Service base;   /* must be first member */
        unsigned radix;
      } Convert_WithRadix;

      /* convert int to string (not really implemented) */
      static void radix_itoa (Convert_Service *service,
                              const A *input,
                              B__Closure closure,
                              void *closure_data)
      {
        char buf[256];
        Convert_WithRadix *wr = (Convert_WithRadix *) service;
        B rv;
        print_int_with_radix (input->val, wr->radix, buf);
        rv.descriptor = &b__descriptor;
        rv.str = buf;
        closure (&rv, closure_data);
      }

      /* convert string to int: use strtoul */
      static void radix_atoi (Convert_Service *service,
                              const B *input,
                              A__Closure closure,
                              void *closure_data)
      {
        Convert_WithRadix *wr = (Convert_WithRadix *) service;
        A rv;
        rv.val = strtoul (input->val, NULL, wr->radix);
        rv.descriptor = &a__descriptor;
        closure (&rv, closure_data);
      }

      /* create a new convert service by radix */
      ProtobufCService *
      create_convert_service_from_radix (unsigned radix)
      {
        Convert_WithRadix *wr = malloc (sizeof (Convert_WithRadix));
        static Convert_Server base_service = CONVERT__INIT(radix_); // creates names radix_atoi and radix_itoa
        wr->base = base_service;
        wr->radix = radix;
        wr->base.base.destroy = (void (*)(ProtobufCService*)) free;
        return (ProtobufCService*) wr;
      }
```

You can cast your service to a `ProtobufCService` object.

Conversely, we generate functions to help you invoke service
methods on generic `ProtobufCService` objects.
These go through the `invoke()` of service
and they work on both services created with create\_service
as well as factory-provided services like those provided by RPC systems.
For example:
```
       void convert__itoa (ProtobufCService *service,
                           const B *input,
                           A__Closure closure,
                           void *closure_data);
```