# Introduction #

protobuf-c's RPC was designed with "main-loop" style concurrency -- meaning we can handle multiple connections, but we handle them by multiplexing on I/O (in the form of file descriptors), rather than spawning new threads.

Currently there are many restrictions for using RPC with multiple threads, but it can be done.  There are also things we wish to do eventually, if they can be done pretty cleanly.

# How to Use a Protobuf-C Server with Threads #
...

# How to Use a Protobuf-C Client with Threads #
...

# TODO: Server #
...

# TODO: Client #
  * blocking client
  * ...