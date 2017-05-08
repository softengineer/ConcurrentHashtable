#ifndef THREADS_H
#define THREADS_H

#include <vector>
#include <utility>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "Common.h"

namespace dt { 
   
    // A class ensuplates pthread_mutex
    class Mutex :noncopyable {
        public :  
            Mutex() {
                if (pthread_mutex_init(&lock, NULL) != 0) {
                    printf("mutex init failed\n");
                }
            }
            
            ~Mutex() {
                pthread_mutex_destroy(&lock);
            }
            
            void tryLock() {
            pthread_mutex_lock(&lock);
            }
            
            void unlock() {
                pthread_mutex_unlock(&lock);
            }
            
        private :
            pthread_mutex_t lock;     
        };

    class Lock {
        public :
            Lock(Mutex * m) : mutex(m) {
                mutex->tryLock();
            }
            ~Lock() {
                mutex->unlock();
            }
        private :
            Mutex * mutex;
    };

   class ReadWriteMutex : noncopyable {
    friend class ReadLock ;
    friend class WriteLock;
   public :
         ReadWriteMutex() {
           pthread_rwlock_init(&lock, NULL);  
        }
        ~ReadWriteMutex() {
          
        }
   private:
       pthread_rwlock_t lock;
   };
   
   class ReadLock : noncopyable {
   public :
       ReadLock(ReadWriteMutex * m): mutex(m) {
          pthread_rwlock_rdlock(&mutex->lock);    
       }
       
       ~ReadLock() {
           pthread_rwlock_unlock(&mutex->lock);
       }
       
   private :
       ReadWriteMutex * mutex;
   };
   
   class WriteLock : noncopyable {
    public:
        WriteLock(ReadWriteMutex * m): mutex(m) {
            pthread_rwlock_wrlock(&mutex->lock);    
        }
        
        ~WriteLock() {
            pthread_rwlock_unlock(&mutex->lock);
        }
        
    private :
        ReadWriteMutex * mutex;
   };

   static void timerHandler( int sig, siginfo_t *si, void *uc );

   struct TimerCall{
       timer_t * _timer_t;
       void (* func) (void *) ;
       void * para ;
   };
   
   ///In linux, timer is one per process, thus
   /// define it as a singleton class, for defining
   /// multiple timers, try call create timer 
   class Timer : noncopyable{
        private :
            
            Mutex * mutex;
            Timer() {
                //mutex = new Mutex();
            }   
            static void timerHandler(int sig, siginfo_t *si, void *uc);
        public :
            static Timer& getInstance()
            {
                static Timer INSTANCE;
                return INSTANCE;
            }
        
        timer_t create(long expireMS, int intervalMS, void (* callbackFunc)(void *), void * para);
        
        void remove(timer_t & timerID);
        
        int make(timer_t *timerID, int expireSecond, int intervalSecond )
        {
            //sigset_t mask;
            struct sigevent         te;
            struct itimerspec       its;
            struct sigaction        sa;
            int                     sigNo = SIGRTMIN;

            /* Set up signal handler. */
            sa.sa_flags = SA_SIGINFO;
            sa.sa_sigaction = timerHandler;
            sigemptyset(&sa.sa_mask);
            if (sigaction(sigNo, &sa, NULL) == -1)
            {
                perror("sigaction");
            }

            /* Set and enable alarm */
            te.sigev_notify = SIGEV_SIGNAL;
            te.sigev_signo = sigNo;
            te.sigev_value.sival_ptr = timerID;
            timer_create(CLOCK_REALTIME, &te, timerID);

            its.it_interval.tv_sec = intervalSecond;
            its.it_interval.tv_nsec = 0;
            its.it_value.tv_sec = expireSecond ;
            its.it_value.tv_nsec = 0;
            timer_settime(*timerID, 0, &its, NULL);

            return 1;
        }
    
        ~Timer() {
            delete mutex;
        }
      };
    
}
#endif // THREADS_H
