#pragma once
#include "ITimer.h"
#include <unordered_map>
#include <map>

#include <boost/pool/pool_alloc.hpp>

namespace BluesTrading
{
    class FakeTimerProvider :public ITimerProvider
    {
    public:
        FakeTimerProvider()
           : current_date(0), current_time(0) , nexttick_time(0)
        {
        }
        struct TimerInfo
        {
            ITimerConsumer* consumer;
            uint32_t eventid;
            uint32_t nextTriggerTime;
            uint32_t interval;
            bool isRepeat;
        };

    public:
        virtual void registerTimerConsumer(ITimerConsumer* consumer) override;
        virtual bool setTimer(ITimerConsumer* consumer, uint32_t eventID, uint32_t timeInMSFromNow, bool repeat) override;
        virtual bool cancelTimer(ITimerConsumer* consumer, uint32_t eventID) override;
      
        virtual uint32_t getCurrentTimeMsInDay() override;  
        virtual uint32_t getCurrentDate() override;  

         // onTimer may call from this thread
        void setNextTickTime(uint32_t timeInMs);  


        void startDate(uint32_t a_date);
        void endDate(uint32_t date);

        /*void invokeAllTimerOnce();*/
        void advanceToTime(uint32_t timeInMs);

       
    private:
        uint32_t current_date;
        uint32_t current_time;
        uint32_t nexttick_time;


        TimerInfo* getNextTimer();

        typedef std::map<uint32_t, TimerInfo*> ConsumerTimerMap;
        //key is event

        std::map<ITimerConsumer* , ConsumerTimerMap>  allTimers;
    };


}