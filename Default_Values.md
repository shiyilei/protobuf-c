# Introduction #

Default values are allowed for required and optional fields.  This document describes their operational semantics.

# Default-Values for Required Fields #

The default value is used for initializing a fresh message.  So the `MESSAGE__INIT` macro and `message__init()` function set the default values.

A required field is always serialized, even if it is set to the default value.

Strings and bytes have their default values stored as globals; occasionally you can use this to see if the value is still the default.  For example,

```
   message Foo
   {  required string a = 1 [default = "blah"];
      required bytes b  = 2 [default = "\0 char"]; }
```

will yield some exposed globals in the generated C code (from the generated header file):
```
   extern char foo__a__default_value[];
   extern uint8_t foo_b__default_value_data[]
   #define FOO__INIT { \
             PROTOBUF_C_MESSAGE_INIT(...), \
             foo__a__default_value, { 6, foo_b__default_value_data } }
```

So if you want to see if `a` is not set to the default value (for example, you may wish to free only non-default values), use:
```
     if (foo.a != foo__a__default_value)
       ... handle non-default value ...
```

# Default-Values for Optional Fields #
Once again, the value is used for initializing a fresh instance, and it can be seen if you deserialize a message missing an optional field.

Except for string and messages, all optional fields have a corresponding `has_field` member of the generated C struct.  In the case of being set to a default value, the `has_field` member will NOT be set, but the "field" member itself will be set to the default value.  Such values will only be serialized if the `has_field` flag is set.

So, if you had a message like:
```
   message Foo {
     optional int32 a = 1 [default = 42];
   }
```
then the corresponding `___INIT` macro would be:
```
   #define FOO__INIT { \
             PROTOBUF_C_MESSAGE_INIT(...), \
             0,       /* !has_a */ \
             42       /* a */ \
           }
```

If you pack the default value of this message, it will be 0 bytes.  To force it to pack the value of `a`, use code like:
```
   Foo foo = FOO__INIT;
   foo.has_a = 1;
   ... pack foo ...
```

For back compatibility, the situation with strings is more annoying.  You see, strings have no corresponding `has_field` member, so the situation must be inferred from the string itself.  If the string is `NULL` or is set to the default-value, it will not be serialized.

To determine if a string is set (and will therefore be serialized), use code like:
```
   if (foo.a != NULL && foo.a != foo__a__default_value)
     {
        ... foo.a is set to a non-default value ...
     }
```

# Future Work #
There are some kinds of default values not supported by protobuf itself.  If protobuf is updated to support these kinds of default values, when we may need to follow suit.
  * default values are ignored for 'repeated' fields
  * default values are not specifiable for sub-message fields.