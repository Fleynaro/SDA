#pragma once

/** Use to init the clock */
#define TIMER_INIT \
    LARGE_INTEGER frequency; \
    LARGE_INTEGER t1,t2; \
    double elapsedTime; \
    QueryPerformanceFrequency(&frequency);


/** Use to start the performance timer */
#define TIMER_START QueryPerformanceCounter(&t1);

/** Use to stop the performance timer and output the result to the standard stream. Less verbose than \c TIMER_STOP_VERBOSE */
#define TIMER_STOP(name) \
    QueryPerformanceCounter(&t2); \
    elapsedTime = (float)(t2.QuadPart - t1.QuadPart) * 1000.0 / double(frequency.QuadPart); \
    std::cout << #name << ": " << elapsedTime << " ms\n";