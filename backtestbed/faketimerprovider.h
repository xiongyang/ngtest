#pragma once
#include "ITimer.h"
#include <unordered_map>
#include <map>

namespace BluesTrading
{
    class FakeTimerProvider :public ITimerProvider
    {
    public:
        virtual bool setTimer(ITimerConsumer* consumer, uint32_t eventID, uint32_t timeInMSFromNow, bool repeat) override;
        virtual bool cancelTimer(ITimerConsumer* consumer, uint32_t eventID) override;
      
        virtual uint32_t getCurrentTimeMsInDay() override;  
        virtual uint32_t getCurrentDate() override;  

         // onTimer may call from this thread
        void setNextTickTime(uint32_t timeInMs);  

         // onTimer may call from this thread
        void setDate(uint32_t a_date);

        void invokeAllTimerOnce();
    private:
        uint32_t current_date;
        uint32_t current_time;
        uint32_t nexttick_time;

        struct TimerInfo
        {
            uint32_t nextTriggerTime;
            uint32_t interval;
            bool isRepeat;
        };

        typedef std::map<uint32_t, TimerInfo> ConsumerTimerMap;
        //key is event

        std::map<ITimerConsumer* , ConsumerTimerMap>  allTimers;
    };


}