#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include <string>
#include <iostream>
#include <map>
#include <set>


class MyUtil
{
public:
    static void msleep(long millis)
    {
        struct timespec tv;
        tv.tv_sec = millis / 1000;
        tv.tv_nsec = (millis % 1000) * 1000000;
        nanosleep(&tv, 0);
    }

    static long long str2ll( const char *str )
    {
        return atoll(str);
    }

    static unsigned long long getNowMs()
    {
        struct timeval tv;
        gettimeofday(&tv, 0);
        return tv.tv_sec * 1000ULL+tv.tv_usec/1000;
    }
};

/*
 * int test()
 * {
 *      TimeCount tc;
 *      tc.begin();
 *      func1();
 *      tc.end();
 *      cout << "cost:" << tc.countSec() << endl;
 * }
 */
class TimeCount
{
public:
    TimeCount()
    {
        m_tBegin.tv_sec  = 0;
        m_tBegin.tv_usec = 0;

        m_tEnd.tv_sec  = 0;
        m_tEnd.tv_usec = 0;
    }

    ~TimeCount(){}
public:
    void begin()
    {
        gettimeofday(&m_tBegin,0);
    }

    void end()
    {
        gettimeofday(&m_tEnd, 0);
    }

    int countMsec()
    {
        return (int)((m_tEnd.tv_sec - m_tBegin.tv_sec)*1000 + (m_tEnd.tv_usec -m_tBegin.tv_usec)/1000.0);
    }

    int countUsec()
    {
        return (m_tEnd.tv_sec - m_tBegin.tv_sec)*1000000+(m_tEnd.tv_usec -m_tBegin.tv_usec);
    }

    int countSec()
    {
        return (m_tEnd.tv_sec - m_tBegin.tv_sec);
    }

public:
    timeval m_tBegin;
    timeval m_tEnd;
};

