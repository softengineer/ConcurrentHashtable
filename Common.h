#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>      /* printf */
#include <time.h>       /* time_t, time (for timestamp in second) */
#include <sys/timeb.h>  /* ftime, timeb (for timestamp in millisecond) */
#include <sys/time.h>   /* gettimeofday, timeval (for timestamp in microsecond) */


namespace tcpp { 
        
        typedef long long int timemilliseconds;
        
        class noncopyable {
        public:
            noncopyable() {}

        private:
            noncopyable(const noncopyable&);
            noncopyable& operator=(const noncopyable&);
        };

        #define DISALLOW_COPY_AND_ASSIGN(TypeName) \
        TypeName(const TypeName&) ;      \
        void operator=(const TypeName&);
        
        timemilliseconds getMilliseconds();

}
#endif // COMMON_H
