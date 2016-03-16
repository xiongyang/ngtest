#pragma once
#include <string>
#include <stdio.h>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "OrderData.h"
#include "boost/filesystem.hpp"

namespace BluesTrading
{

    std::string getTimeInMSStr(uint32_t timeInMS);

    const std::string getTimeStr(uint32_t timeInMS);

    void printOrder(std::ostream& of, const OrderDataDetail& order);

   template<typename ProcessType>
   void traverseDir(const std::string& dirName, ProcessType& process);

   template<typename MapType, typename pred>
   void remove_if_map(MapType&& c, pred&& predfun);

   std::vector< std::unordered_map<std::string, std::string> > parserProps(std::istream& inputStr);


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

#include "util.inl"

