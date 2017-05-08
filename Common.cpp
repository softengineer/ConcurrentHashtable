#include "Common.h"

dt::timemilliseconds dt::getMilliseconds() {
           /* Example of timestamp in second. */
            time_t timestamp_sec; /* timestamp in second */
            time(&timestamp_sec);  /* get current time; same as: timestamp_sec = time(NULL)  */
           
            /* Example of timestamp in millisecond. */
            struct timeb timer_msec;
            timemilliseconds timestamp_msec; /* timestamp in millisecond. */
            if (!ftime(&timer_msec)) {
                timestamp_msec = ((long long int) timer_msec.time) * 1000ll + 
                                    (long long int) timer_msec.millitm;
            }
            else {
                timestamp_msec = -1;
            }
            printf ("%lld milliseconds since epoch\n", timestamp_msec);
            return  timestamp_msec;
} 
