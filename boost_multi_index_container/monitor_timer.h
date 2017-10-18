#include <tchar.h>  
#include <cstdlib>  
#include <iostream>  
#include <sys/timeb.h>  
#include <ctime>  
#include <climits> 
#include <iostream>

using namespace std;

struct timeb startTime, endTime;

#define START_MONITOR_TIMER()   {ftime(&startTime);}
#define STOP_MONITOR_TIMER()    {ftime(&endTime);cout << "TIME usage: "<< (endTime.time - startTime.time) * 1000 + (endTime.millitm - startTime.millitm) << "ms"<< endl;}

class Monitor_Timer
{
   private:
    struct timeb startTime;
    struct timeb endTime;
    float        time_in_ms;
    float        time_in_s;

   public:
    Monitor_Timer()
    {
        time_in_ms = 0.0;
        time_in_s  = 0.0;
    }
    ~Monitor_Timer() {}

   public:
    void
    start_timer()
    {
        ftime(&startTime);
    }

    void
    stop_timer()
    {
        ftime(&endTime);
        time_in_ms = (endTime.time - startTime.time) * 1000 + (endTime.millitm - startTime.millitm);
        time_in_s  = time_in_ms / 1000.00;
    }

    float get_time_ms()
    {
        cout << "TIME usage: " << time_in_ms << "ms" << endl;
        return time_in_ms;
    }

    float get_time_s()
    {
        cout << "TIME usage: " << time_in_s << "s" << endl;
        return time_in_s;
    }
};