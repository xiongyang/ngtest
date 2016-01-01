#include "faketimerprovider.h"

namespace BluesTrading
{

    bool FakeTimerProvider::setTimer(ITimerConsumer* consumer, uint32_t eventID, uint32_t timeInMSFromNow, bool repeat)
    {
        uint32_t triggerTime = timeInMSFromNow + current_time;
        TimerInfo info
        {
                triggerTime,
                timeInMSFromNow,
                isRepeat
        };
        return allTimers[consumer].insert(std::make_pair(eventID, info)).second;
    }

    bool FakeTimerProvider::cancelTimer(ITimerConsumer* consumer, uint32_t eventID)
    {
       return allTimers[consumer].erase(eventID).second;
    }


    std::uint32_t FakeTimerProvider::getCurrentTimeMsInDay()
    {
        return current_time;
    }

    std::uint32_t FakeTimerProvider::getCurrentDate()
    {
        return current_date;
    }

    void FakeTimerProvider::setNextTickTime(uint32_t timeInMs)
    {
       // current_time = timeInMs;

        //for (auto consumerMap_pair : allTimers)
        //{
        //    std::map<uint32_t, TimerInfo> triggerlist;

        //    for (auto timerMapItem: consumerMap_pair->second)
        //    {
        //        if (timerMapItem->second.nextTriggerTime >= timeInMs)
        //        {
        //            triggerlist.insert(timerMapItem->second.nextTriggerTime, timerMapItem->second);
        //        }
        //    }


        //}
    }

    void FakeTimerProvider::setDate(uint32_t a_date)
    {
        if (a_date != current_date)
        {
            invokeAllTimerOnce();
            current_date = a_date;
        }
    }

}


