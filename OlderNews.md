# Old News #

## Jun 11, 2009 ##
**protobuf-c 0.11** is released.
  * mostly compilation fixes for new versions of g++ and protobuf, and a few other compilation issues (e.g. const correctness)  (Many thanks to dcreager, XXX name others)
  * handle a few integer overflow situations a little better (unlikely to occur -- requires unpacking a huge message)
  * no binary or source incompatibilities -- don't bother upgrading unless you are experiencing build problems

## Apr 4, 2009 ##
**protobuf-c 0.10** is released (don't ask about 0.9...)
  * several bug fixes for corruption and build issues
  * numerous fixes to the RPC system
  * **generated source-incompatibility for service/rpc users**:  you must re-run protoc-c (those using 0.9 or 10beta already have the new code-generator).
  * credits: (thanks!)
    * Jason Lunz for his allocation failure fixes (including excellent [test code](http://code.google.com/p/protobuf-c/source/diff?spec=svn173&r=173&format=side&path=/trunk/src/test/test-generated-code2.c))
    * Landon Fuller for his corruption [test-cases](http://code.google.com/p/protobuf-c/issues/detail?id=16&can=1) and [fixes](http://code.google.com/p/protobuf-c/source/diff?spec=svn181&r=181&format=side&path=/trunk/src/google/protobuf-c/protobuf-c.c).

## Jan 29, 2009 ##
**protobuf-c 0.8** is released
  * bugs fixed in service invocation; handle NULL in certain required fields (issues 12 and 13)
  * An [RPC\_System](RPC_System.md) is included, with tests and examples.  It's in beta, so be a little careful.

## Dec 17, 2008 ##
**protobuf-c 0.7** is released.  This is a bug fix release, it does not break any compatibility.

Bugs fixed:
  * fixes minor compilation issues (see issues 10 and 11 in the tracker)
  * fixes a long-standing memory leak where unknown fields are not freed by `free_unpacked()`

## Nov 30, 2008 ##

**protobuf-c 0.6** is released.  It breaks binary and generated-source compatibility:  you must re-run protoc-c to generate new .pb-c.h and .pb-c.c files.

The new features of 0.6 are:
  * [default values](Default_Values.md) now set correctly
  * For each message type, we generate a `message__init(Message *)` function.  This function never allocates memory, and is equivalent to using the `MESSAGE__INIT` macro.