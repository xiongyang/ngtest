#pragma  once
#include <cstdint>

namespace BluesTrading
{
    class ITimerConsumer
    {
    public:
        virtual ~ITimerConsumer(){}
        virtual void onTimer(uint32_t eventID, uint32_t currentTime) {};
        virtual void onStartDay(uint32_t date) {};
        virtual void onEndDay(uint32_t date) {};
    };

    class ITimerProvider
    {
    public:
        virtual ~ITimerProvider(){}
        virtual void registerTimerConsumer(ITimerConsumer* consumer) = 0;   
        virtual bool setTimer(ITimerConsumer* consumer, uint32_t eventID, uint32_t timeInMSFromNow, bool repeat) = 0;
        virtual bool cancelTimer(ITimerConsumer* consumer, uint32_t eventID) = 0;
       
        virtual uint32_t getCurrentTimeMsInDay() = 0;  
        virtual uint32_t getCurrentDate() = 0;  //YYYYMMDD
    };

}
