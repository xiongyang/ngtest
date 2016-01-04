#pragma once
#include <string>
#include <stdio.h>
#include <functional>

namespace BluesTrading
{

    std::string getTimeInMSStr(uint32_t timeInMS) {
        char pTimeBuf[64];
        int milliSecond = timeInMS % 1000;
        int second = (timeInMS - milliSecond) / 1000; second = second % 60;
        int minute = ((timeInMS - milliSecond) / 1000 - second) / 60; minute = minute % 60;
        int hour = (((timeInMS - milliSecond) / 1000 - second) / 60 - minute) / 60;

        sprintf(pTimeBuf, "%02d:%02d:%02d.%03d", hour, minute, second, milliSecond);
        return pTimeBuf;
    }


    class ScopeGuard
    {
    public:
        explicit ScopeGuard(std::function<void()> onExitScope) 
            : onExitScope_(onExitScope), dismissed_(false)
        { }

        ~ScopeGuard()
        {
            if(!dismissed_)
            {
                onExitScope_();
            }
        }

        void Dismiss()
        {
            dismissed_ = true;
        }

    private:
        std::function<void()> onExitScope_;
        bool dismissed_;

    private: // noncopyable
        ScopeGuard(ScopeGuard const&);
        ScopeGuard& operator=(ScopeGuard const&);
    };


}


