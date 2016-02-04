# Introduction #

Core `protobuf-c` is thread-neutral:  it never modifies any structures that can potentially be shared.

Well, you are responsible for certain parts.  For example, we assume that your allocator can be used without any sort of locking (which is true for `malloc()`, `free()` and `realloc()` - so the default allocator should be fine.)

The RPC system is not:  however, it can be used in many multithreaded environments as described below.

# Good News #
Core `protobuf-c` is thread-safe.

## Fine print ##
Well, if you supply an allocator that you use in multiple threads, it must be thread-safe.  That works fine for the default allocators (presuming that `malloc`, `realloc` and `free` are thread-safe).  If you are providing a custom allocator that you use in multiple threads, it has to be itself thread-safe.

The core of the [Services](Services.md) implementation is thread-safe.  However, most of the burden of thread-safety is thrust upon the implementations of `ProtobufCService`.

# RPC #
Of course single-threaded apps are ok.  Many protocols could be implemented within the framework of the dispatch - that is one easy way to handle things.

Another, probably the best, paradigm, is to run `protobuf-c`'s dispatch in a single thread, and allow `protobuf-c` to proxy your response from a different thread.

See our [RPC\_Threading](RPC_Threading.md) page for more details.