# ConcurrentHashtable

ConcurrentHashtable is a Hashtable tested under Linux platform, which provide the following features

1. A C++ hashtable can work under the multiple thread mode
2. Each element in hashtable is with a timestamp, which supports expired. It supports callback function for handle expired element.

It is tested under the C98 and g++ 4.8 in Linux.

Compile steps :
1.1 cmake .
1.2 make

To use it, CMake need to be installed in server

2. Circular buffer, which is tested under C++11, is useful under the producer / consumer mode

Enjoy!
