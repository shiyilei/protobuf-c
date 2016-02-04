﻿= Design of protobuf-c =

The overall goal is to keep the code-generator as simple
as possible.  Hopefully performance isn't sacrificed to that end!

Anyways, we generate very little code: we mostly generate
structure definitions (for example enums and structures
for messages) and some metadata which is basically
reflection-type data.

The serializing and deserializing is implemented in a library,
called libprotobuf-c rather than generated code.
