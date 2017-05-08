#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <cstddef>
#include <utility>
#include <stdio.h>

#include "Common.h"
#include "Threads.h"
#include "HashNode.h"

namespace dt { 
         
    const int defaultCapacity = 100;
    const float defaultLoadFactor = 0.75f;
    
    extern timemilliseconds getMilliseconds(void) ;

    template<typename K, typename V, typename F = KeyHash<K> >
    class Hashtable;

    template <typename K, typename V, typename F = KeyHash<K> >
    class Iterator
    {
      public :
        Iterator(Hashtable<K, V, F> & table):hashtable(table),position (0), current(NULL) {       
        }
        
        Iterator (const Iterator & itr):hashtable(itr.hashtable), position(0), current(NULL) {            
        }
        
        void operator = ( const Iterator & itr) {
            hashtable = itr.hashtable;
            position = itr.position;
            current = itr.current;
        }
        
        bool hasNext() {
            ReadLock(hashtable.mutex);
         
            if (position >= hashtable.capacity)
                return false;
            
            if (current != NULL) {
                if (current->getNext() != NULL) {
                    current = current->getNext();
                    
                    return true;
                }
            }
            for (int j = position ; j < hashtable.capacity; j++) {
                HashNode<K, V > * node = hashtable.table[j];
                if (node != NULL) {
                    current = node;
                    position = j + 1;
                    return true;
                }
            }
            
            return false;
        }
        
        void next(K & k, V & v){
            if (current == NULL)
                return;
            else {
                k = current->getKey();
                v = current->getValue();
            }
        } 
        
        void reset() {
           current = NULL;
           position = 0;
        }
        
        private:
            Hashtable<K, V, F> & hashtable;
            HashNode<K, V> * current;
            size_t position;
    };
    
    
    template <typename K, typename V, typename F = KeyHash<K> >
    class ExpiredIterator
    {
      public :
        ExpiredIterator(Hashtable<K, V, F> & table, timemilliseconds & base):hashtable(table),position (0), current(NULL), basetime(base) {
                
        }
        
        ExpiredIterator (const ExpiredIterator & itr):hashtable(itr.hashtable), position(0), current(NULL), basetime(itr.basetime) {
            
        }
        
        void operator = ( const ExpiredIterator & itr) {
            hashtable = itr.hashtable;
            position = itr.position;
            current = itr.current;
        }
        
        bool hasNext() {
            ReadLock(hashtable.mutex);
         
            if (hashtable.periodSeconds == 0)
                return false;
            
            if (position >= hashtable.capacity)
                return false;
            
            if (current != NULL) {
                for (HashNode<K, V> * c = current->getNext(); c != NULL; c = c->getNext()) {
                    if (isExpired(c)) {
                       current = c;
                       return true;
                    }
                }
            }
            for (int j = position ; j < hashtable.capacity; j++) {
                HashNode<K, V > * node = hashtable.table[j];
                for (HashNode<K, V> * c = node; c != NULL; c = c->getNext()) {
                    if (isExpired(c)) {
                        current = c;
                        position = j + 1;
                        return true;
                    }
                }
            }
            
            return false;
        }
        
        
        void next(K & k, V & v){
                if (current == NULL)
                    return;
                else {
                    k = current->getKey();
                    v = current->getValue();
                }
        } 
        
        void reset() {
           current = NULL;
           position = 0;
        }
        
        private:
            
            Hashtable<K, V, F> & hashtable;
            HashNode<K, V> * current;
            timemilliseconds basetime;
            size_t position;
            
            bool isExpired(HashNode<K, V> * node) {
                return basetime - node->getTime() > hashtable.periodSeconds * 1000;
            }
    };
    
    template <typename K, typename V, typename F>
    void expire(void * para);
    
    template <typename K, typename V, typename F >
    class Hashtable : noncopyable
    {
            template <typename X, typename Y, typename Z>
            friend class Iterator;
            
            template <typename X, typename Y, typename Z>
            friend class ExpiredIterator;
        
            template <typename X, typename Y, typename Z>
            friend void expire(void * para);
       public :
            Hashtable(): timerId(NULL), m_size(0), capacity(defaultCapacity), loadFactor(defaultLoadFactor), threshold(defaultCapacity * defaultLoadFactor), periodSeconds(0)
            {
                mutex = new ReadWriteMutex();
                initTable();
            }
            
            Hashtable(int initCapability):timerId(NULL), m_size(0), capacity(initCapability), loadFactor(defaultLoadFactor), threshold(initCapability * defaultLoadFactor), periodSeconds(0)
            {
                mutex = new ReadWriteMutex();
                initTable();
            }
            
            Hashtable(int initCapability, float factor, int p = 0, void (*func)(K &) = NULL):timerId(NULL), m_size(0), capacity(initCapability), loadFactor(factor), threshold(initCapability * factor), periodSeconds(p), expiredFunc(func)
            {
                mutex = new ReadWriteMutex();
                initTable();
                if (periodSeconds == 0)
                    timerId = NULL;
                else {
                ;
                    timerId = Timer::getInstance().create(periodSeconds, periodSeconds, expire<K, V, F>, this);
                }
            }
            
            ~Hashtable()
            {
                clear();
                delete [] table;
                delete mutex;
                if (timerId != NULL) {
                    printf ("timer = %p\n", timerId);
                    Timer::getInstance().remove(timerId);
                }
            }
        
            size_t size()
            {
                ReadLock lock(mutex);
                return m_size;
            }
            
            void        put(const K & key, const V & val)
            {
                WriteLock lock(mutex);
                timemilliseconds mill = getMilliseconds();
                bool update = putEntryInternal(table, capacity, key , val, mill);
                if (!update)
                    m_size += 1;
                if (m_size >= threshold) {
                    size_t newcapacity = capacity <<2;
                    rehash(newcapacity);
                }
            }
            
            bool        get(const K & key, V & val)
            {
                    ReadLock lock(mutex);
                    unsigned long hashValue = hashFunc(key, capacity);
                    HashNode<K, V> *entry = table[hashValue];

                    while (entry != NULL) {
                        if (entry->getKey() == key) {
                            val =  entry->getValue();
                            return true;
                        }

                        entry = entry->getNext();
                    }

                    return false;
            }
            
            bool        contain(const K & key)
            {
                ReadLock lock(mutex);
                unsigned long hashValue = hashFunc(key, capacity);
                HashNode<K, V> *entry = table[hashValue];

                while (entry != NULL) {
                    if (entry->getKey() == key) {
                        return true;
                    }

                    entry = entry->getNext();
                }

                return false;
            }
            
            bool        remove(const K & key)
            {
                WriteLock lock(mutex);
                unsigned long hashValue = hashFunc(key, capacity);
                HashNode<K, V> *prev = NULL;
                HashNode<K, V> *entry = table[hashValue];

                while (entry != NULL && entry->getKey() != key) {
                    prev = entry;
                    entry = entry->getNext();
                }

                if (entry == NULL) {
                    // key not found
                    return false;

                } else {
                    if (prev == NULL) {
                        // remove first bucket of the list
                        table[hashValue] = entry->getNext();

                    } else {
                        prev->setNext(entry->getNext());
                    }

                    delete entry;
                    m_size -= 1;
                    return true;
                }   
            }
            
            void        clear()
            {
                mutex = new ReadWriteMutex();
                for (size_t j = 0; j<capacity; j++)
                {
                    HashNode<K, V> * node = table[j];
                    HashNode<K, V> * prev;
                    while (node != NULL) {
                        prev = node;
                        node = node->getNext();
                        delete prev;
                    }
                }
                m_size = 0;
            }
            Iterator<K, V, F> keys()
            {
                return Iterator<K,V, F>(*this);
            }
            
            ExpiredIterator<K, V, F> expiredKeys()
            {
                 timemilliseconds milliseconds = getMilliseconds();
                return ExpiredIterator<K,V, F>(*this, milliseconds);
            }
            
        private :
            timer_t  timerId;
            ReadWriteMutex * mutex;
            size_t  m_size;
            int     capacity;
            float   loadFactor;
            int     threshold;
            int     periodSeconds;
            F       hashFormula;
            void    (*expiredFunc)(K &);
            // hash table
            HashNode<K, V> ** table ;
            
            void          initTable()
            {
              table = new HashNode<K, V> * [capacity];
              for (size_t i = 0; i<capacity; i++) {
                    table[i] = NULL;
              }   
            }
            
            void  rehash(const size_t newCapacity)
            {
                
                HashNode<K, V> ** newTable = new HashNode<K, V> * [newCapacity];
                for (int j = 0; j< capacity; j++) {
                    HashNode<K, V> * entry = table[j];
                    while (entry != NULL) {
                        putEntryInternal(newTable, newCapacity, entry->getKey(), entry->getValue(), entry->getTime());
                        delete entry;
                        entry = entry->getNext();
                    }
                }
                
                delete [] table;
                table = newTable;
                capacity = newCapacity;
                threshold =  capacity * loadFactor;
            }
            
            bool          putEntryInternal(HashNode<K, V> ** targetTable, const size_t & size, const K & key, const V & val, const timemilliseconds & time)
            {
                 HashNode<K, V> *prev = NULL;
                unsigned long hashValue = hashFunc(key, size);
                HashNode<K, V> *entry = targetTable[hashValue];

                while (entry != NULL && entry->getKey() != key) {
                    prev = entry;
                    entry = entry->getNext();
                }

                if (entry == NULL) {
                    entry = new HashNode<K, V>(key, val, time);

                    if (prev == NULL) {
                        // insert as first bucket
                        targetTable[hashValue] = entry;

                    } else {
                        prev->setNext(entry);
                    }
                } else {
                    // just update the value
                    entry->setValue(val);
                    entry->setTime(time);
                    return true;
                } 
                return false;
            }
            
            unsigned long hashFunc(const K & key, const size_t & size)
            {
                 unsigned long hashVal = hashFormula(key);
                return hashVal % size;
            }
           
    };
    
    template <typename K, typename V, typename F>
    void expire(void * para) {
        Hashtable<K, V, F> * table = (Hashtable<K, V, F> * )para;
        ExpiredIterator<K, V, F> itr = table->expiredKeys();
        while (itr.hasNext()) {
            if (table ->expiredFunc != NULL) {
                K  key;
                V  val;
                itr.next(key, val);
                table->expiredFunc(key);
                
            }
        }
    }
}
#endif // HASHTABLE_H
