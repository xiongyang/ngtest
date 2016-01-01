#pragma  once
#include <cstdint>

namespace BluesTrading
{
    class ITimerConsumer
    {
    public:
        virtual ~ITimerConsumer(){}
        virtual void onTimer(uint32_t eventID, uint32_t currentTime) = 0;
    };

    class ITimerProvider
    {
    public:
        virtual ~ITimerProvider(){}
        virtual bool setTimer(ITimerConsumer* consumer, uint32_t eventID, uint32_t timeInMSFromNow, bool repeat) = 0;
        virtual bool cancelTimer(ITimerConsumer* consumer, uint32_t eventID) = 0;
       
        virtual uint32_t getCurrentTimeMsInDay() = 0;  
        virtual uint32_t getCurrentDate() = 0;  
    };

}
