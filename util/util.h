#pragma once
#include <string>
#include <stdio.h>
#include <functional>
#include <iostream>
#include "OrderData.h"
#include "boost/filesystem.hpp"

namespace BluesTrading
{

    std::string getTimeInMSStr(uint32_t timeInMS);
    void printOrder(std::ostream& of, const OrderDataDetail& order);

   template<typename ProcessType>
   void traverseDir(const std::string& dirName, ProcessType& process)
   {
       if (!boost::filesystem::exists(dirName))
       {
           std::cout << "dir not exists" << dirName;
           return;
       }

       if (boost::filesystem::is_directory(dirName))
       {
           for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(dirName))
           {
               if (boost::filesystem::is_regular_file(x))
               {
                      process(x.path().string());
                      //tickDataStore.push_back(MarketDataStore(x.path().string()));
               }
               else if(boost::filesystem::is_directory(x))
               {
                   traverseDir(x.path().string(), process);
               }
           }
       }
       else if (boost::filesystem::is_regular_file(dirName))
       {
           process(dirName);
         //  tickDataStore.push_back(MarketDataStore(dirName));
       }
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

    template<typename MapType, typename pred>
    void remove_if_map(MapType&& c, pred&& predfun)
    {
        for(auto iter = c.begin(); iter != c.end(); ++ iter)
        {
            if (predfun(*iter))
            {
                c.erase(iter++);
            }
        }
    }

     const std::string getTimeStr(uint32_t timeInMS);

}


