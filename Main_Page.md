#summary protobuf-c wiki home page

# Introduction #

protobuf-c is a C binding for Google's [Protocol Buffers](http://code.google.com/p/protobuf/) (aka protobuf) library and code generator.

The overall [design](Design.md) is similar to the C++ structure,
except we try to generate as little code as possible.

# Compilation and Installation #
This is an automake project, so to build it use something like:
```
   ./configure --prefix=$HOME/install
   make
   make install
```
To install into `/usr` like a normal package would, use `--prefix=/usr`.

# Getting Started #
It might be easiest to learn with a few [examples](Examples.md).

To use the system, you should read more
about [the code we generate](Generated_Code.md).
To understand how the generated code works,
read about [libprotobuf\_c](libprotobuf_c.md).

# Status #
  * The message serialization and deserialization code has been thoroughly tested.
  * Still designing the Service stuff, so be careful about using it.
  * Need to decide what to do about extensions
  * Need to decide whether to handle groups.

# Caveats #

Some binary compatibility changes are expected until version 1.0 is released; so make sure you are linking to the protobuf-c that corresponds to the generated code you are using!

# TODO items #
Currently, these are in a file called [TODO](http://code.google.com/p/protobuf-c/source/browse/trunk/TODO) in the source package.