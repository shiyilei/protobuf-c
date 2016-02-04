# Current Suggestion #
Extensions are not implemented.  I recommend merely mechanically adding the extension values to the message descriptor before running protoc-c.  It works well, and isn't very hard.

# Use Cases #
VOLUNTEERS? uh, yeah, please write some example use-cases if you are interested

# THIS IS AN UNIMPLEMENTED PROPOSAL #

# Introduction #

This is a DRAFT (if that) of the extension API.

The (new as of May 16) plan is to make a MessageDescriptor for every namespace/message combination that has extensions.  The lexically-containing message name (if any) may need to be shoved somewhere.  We may add an "extends" field to MessageDescriptor to clarify this purpose.

# libprotobuf-c functions #
```
  ProtobufCMessage * protobuf_c_extension_unpack (ProtobufC_Message *msg,
                                                        ProtobufC_MessageDescriptor   *extension,
                                                        ProtobufC_Allocator *allocator);
```
If any extension is tagged as "required", its absence will cause the unpacking to fail.  You will also get failures on type errors.

For the packing side, you can pack the extensions (using get\_packed\_size/pack or pack\_buffer) and concatenate as binary data.  Later, we may add support to generate "UnknownFields" so that you can incorporate them into a message - it's not clear that that's useful.

# Generated code #
```
   extern ProtobufC_MessageDescriptor extension_namespace___extends___full_message_name;
   ExtensionNamespace___extends___FullMessageName *extension_namespace___extends___full_message_name__unpack(FullMessageName *orig_message,
                                                        ProtobufC_Allocator *allocator);
                
```