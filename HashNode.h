#ifndef HASHNODE_H
#define HASHNODE_H

#include <cstddef>
#include "Common.h"

namespace dt { 
    
    // Hash node class template
    template <typename K, typename V>
    class HashNode : noncopyable
    {
    public:
    
        HashNode(const K &key, const V &value, const timemilliseconds & t) :
            _key(key), _value(value),time(t),  _next(NULL)
        {
        }

        K getKey() const
        {
            return _key;
        }

        V getValue() const
        {
            return _value;
        }

        void setValue(V value)
        {
            _value = value;
        }

        HashNode *getNext() const
        {
            return _next;
        }

        void setNext(HashNode *next)
        {
            _next = next;
        }
        
        void setTime(long newTime) {
            time = newTime;
        }
        
        timemilliseconds getTime() {
            return time;
        }

    private:
    // key-value pair
        K _key;
        V _value;
        timemilliseconds time;
        // next bucket with the same key
        HashNode *_next;
        bool operator==(const HashNode& other) const;
    };

    // Default hash function class
    template <typename K>
    struct KeyHash {
        unsigned long operator()(const K &key) const
        {
            unsigned long val =  reinterpret_cast<unsigned long  >(key) ;
            return val;
        }
    };
}
#endif // HASHNODE_H
