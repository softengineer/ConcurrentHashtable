# ConcurrentHashtable

It includes some useful data structure which can work under the multiple threads environment

1. A C++ hashtable can work under the multiple thread mode, beside, it is allowed to timeout when the elements in hashtable are exceed the threshold you set, which is suit for cache.

It is tested under the C98 and g++ 4.8 in Linux.

1.1 cmake .
1.2 make

To use it, CMake need to be installed in server

2. Circular buffer, which is tested under C++11, is useful under the producer / consumer mode

Enjoy!
