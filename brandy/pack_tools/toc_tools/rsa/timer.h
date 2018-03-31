#ifndef _QT_TIMER_H_INCLUDED_
#define _QT_TIMER_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#ifdef WIN32
#include <windows.h>
#else

#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>

#endif
//for LARGE_INTEGER, QueryPerformanceCounter()
//and QueryPerformanceFrequency()
#ifdef WIN32

class Timer  
{
    private:
        LARGE_INTEGER begin_;
        LARGE_INTEGER end_;
        LARGE_INTEGER counter_;
        bool running_;

    public:
        static unsigned long frequency(void)
        {
            LARGE_INTEGER frqc;
            ::QueryPerformanceFrequency(&frqc);
            return frqc.LowPart;
        }
        Timer(void): running_(false)
        {
            begin_.QuadPart = end_.QuadPart = counter_.QuadPart = 0;
        }
        void start(void)   //开始计时 
        {
            if(running_ == false){
	           running_ = true;
	           ::QueryPerformanceCounter(&begin_);
            }
        }
        void stop(void)   //结束计时 
        {
            if(running_ == true){
	           ::QueryPerformanceCounter(&end_);
	           running_ = false;
	           counter_.QuadPart += end_.QuadPart - begin_.QuadPart;
            }
        }
        void reset(void)   //清零从新计时 
        {
	        begin_.QuadPart = end_.QuadPart = counter_.QuadPart = 0;
	        running_ = false;
        }
        double seconds(void) //计算时间 
        {
            LARGE_INTEGER frqc;
            ::QueryPerformanceFrequency(&frqc);
            if(running_ == false)
                return counter_.QuadPart / static_cast<double>(frqc.QuadPart);
            ::QueryPerformanceCounter(&end_);
            return counter_.QuadPart + (end_.QuadPart - begin_.QuadPart)
            / static_cast<double>(frqc.QuadPart);
        }
        bool running(void) const
        {
            return running_; 
        }
};//~class Timer
#else

/*
timeval

struct  timeval{
       long  tv_sec; //秒
       long  tv_usec;//微妙
}；

timezone 结构定义为：
struct  timezone{
        int tz_minuteswest; //和greenwich 时间差了多少分钟
        int tz_dsttime;     //type of DST correction
}
*/
class Timer  
{
    private:
    	struct  timeval    begin_;
    	struct  timeval    end_;
    	struct  timeval    counter_;    	
		struct  timezone   tz;
	
       
        bool running_;

    public:
        static unsigned long frequency(void)
        {
        	return 0;       	

        }
        Timer(void)
        {
        	memset(&begin_, 0x00, sizeof(timeval));
        	memset(&end_, 0x00,sizeof(timeval));
        	memset(&counter_, 0x00,sizeof(timeval));        	
      		running_  = false;
            //begin_.QuadPart = end_.QuadPart = counter_.QuadPart = 0;
        }
        void start(void)   //开始计时 
        {
            if(running_ == false){
	           running_ = true;
	           //::QueryPerformanceCounter(&begin_);
	           gettimeofday(&begin_,&tz);
            }
        }
        void stop(void)   //结束计时 
        {
            if(running_ == true){
	          // ::QueryPerformanceCounter(&end_);
	          gettimeofday(&end_,&tz);
	           running_ = false;
	          // counter_.QuadPart += end_.QuadPart - begin_.QuadPart;
	          counter_.tv_usec = end_.tv_usec - begin_.tv_usec;
	          counter_.tv_sec = end_.tv_sec - begin_.tv_sec;
	          
            }
        }
        void reset(void)   //清零从新计时 
        {
	       // begin_.QuadPart = end_.QuadPart = counter_.QuadPart = 0;
		    memset(&begin_, 0x00, sizeof(timeval));
        	memset(&end_, 0x00,sizeof(timeval));
        	memset(&counter_, 0x00,sizeof(timeval));
	        running_ = false;
        }
        double seconds(void) //计算时间 
        {
           /* LARGE_INTEGER frqc;
            ::QueryPerformanceFrequency(&frqc);
            if(running_ == false)
                return counter_.QuadPart / static_cast<double>(frqc.QuadPart);
            ::QueryPerformanceCounter(&end_);
            return counter_.QuadPart + (end_.QuadPart - begin_.QuadPart)
            / static_cast<double>(frqc.QuadPart);
            */
            return counter_.tv_usec;
        }
        bool running(void) const
        {
            return running_; 
        }
};//~class Timer
#endif // WIN32

#endif //~ #infdef _QT_TIMER_H_INCLUDED_
//~Timer.h
