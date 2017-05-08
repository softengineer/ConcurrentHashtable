//
//  CircularBuffer.h
//

#pragma once

#include <thread>


namespace dt {
// class implements circular buffer
// refer to https://en.wikipedia.org/wiki/Circular_buffer

template <typename T, unsigned int max>
class CircularBuffer
{
public:
    
    CircularBuffer() : m_Size(0),  m_PushWait(false), m_PullWait(false), m_WriteOffset(0), m_ReadOffset(0)
    {}
    ~CircularBuffer(){};
    
    /**
     * @name    push
     * @brief   push a element into circular.
     *
     * Multiple threads operation on this method is supported
     * When circularBuffer is full, thread on this method will be blocked
     * until circularBuffer is not full
     *
     *
     * @param [in] val a inserted value to circular buffer.
     *
     * @retval void
     *
     * Example Usage:
     * @code
     @    CircularBuffer<int, 10> buff;
     *    buff.push(10);
     * @endcode
     */
    void push(const T & val )
    {
        Mutex guard( m_Mutex );
        if (m_Size == MAX_SIZE)
        {
            //buffer is full, push thread need wait
            setWaitCondition(m_PushWait, guard);
        }
        
        notifyWaitCondition(m_PullWait);
        m_WriteOffset = (m_WriteOffset + 1) % MAX_SIZE;
        m_Buffer[m_WriteOffset] = val;
        m_Size++;
        
    }
    
    
    /**
     * @name    pull
     * @brief   pull a element from circular.
     *
     * This API provides certain actions as an example.
     *
     * Multiple threads operation on this method is supported
     * When circularBuffer is empty, thread on this method will be blocked
     * until circularBuffer is push with element
     
     * @retval T val : the value from circularBuffer
     *
     * Example Usage:
     * @code
     @    CircularBuffer<int, 10> buff;
     *    int val = buff.pull();
     * @endcode
     */
    T & pull()
    {
        Mutex guard( m_Mutex );
        if ( m_Size == 0)
        {
            // buffer is empty, pull thread need wait
            setWaitCondition(m_PullWait, guard);
        }
        
        notifyWaitCondition(m_PushWait);
        m_Size--;
        m_ReadOffset = (m_ReadOffset + 1) % MAX_SIZE;
        return m_Buffer[m_ReadOffset];
        
    }
    
    /**
     * @name    size
     * @brief   get elment size of circular.
     *
     *
     * @retval size of circular buffer
     *
     * Example Usage:
     * @code
     @    CircularBuffer<int, 10> buff;
     *    size_t size = buff.size();
     * @endcode
     */
    size_t size()
    {
        Mutex guard( m_Mutex );
        return m_Size;
    }
    
private:
    size_t                    m_Size;
    std::mutex                m_Mutex;
    std::condition_variable   m_cond;
    
    bool m_PushWait; // true when push thread is waiting, size = MAX_SIZE
    bool m_PullWait; // true when pull thread is waiting, size = 0
    
    enum { MAX_SIZE = max };
    T  m_Buffer[MAX_SIZE];
    uint32_t m_WriteOffset;
    uint32_t m_ReadOffset;
    
    void notifyWaitCondition(bool & waitState)
    {
        if (waitState)
        {
            waitState = false;
            m_cond.notify_one();
        }
    }
    
    void setWaitCondition(bool & waitState, Mutex & mutex)
    {
        waitState = true;
        m_cond.wait(mutex);
    }
};
}
