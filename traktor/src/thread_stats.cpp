#include <atomic>
#include <thread>
#include <chrono>

#include "stats.h"
#include "shared.h"

/*
float ns_to_ms_per_fps(uint64 ns, int fps)
{
    return fps > 0 ? 
                  ( ( (float)ns / (float)fps ) / (float)1000000.0 ) 
                : -1;
}
*/

void thread_stats(const std::atomic_bool& shutdown_requested, Stats& stats)
{
    for (;;)
    {
        std::this_thread::sleep_for( stats.pause );

        stats.tick();

        if ( shutdown_requested.load() )
        {
            break;
        }
    }
}
