#include "Hashtable.h"


void func1(unsigned long & s ){
    
}

int main() {
   
     tcpp::Timer & timer = dpi::Timer::getInstance();
     tcpp::Hashtable<unsigned long, unsigned long> table (100, 0.75f, 5, func1);
     
     while (true)
     {
         sleep(1);
    }
}
