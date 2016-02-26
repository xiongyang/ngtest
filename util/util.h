#pragma once
#include <string>
#include <stdio.h>
#include <functional>
#include <iostream>
#include "OrderData.h"


namespace BluesTrading
{

    std::string getTimeInMSStr(uint32_t timeInMS);
    void printOrder(std::ostream& of, const OrderDataDetail& order);

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


