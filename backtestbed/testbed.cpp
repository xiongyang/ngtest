#include <fstream>
#include <iostream>
#include "boost/filesystem.hpp"

#include "testbed.h"
#include "dynamicloader.h"
#include <chrono>

namespace BluesTrading
{

    void TestBed::Init(const std::string& data , const std::string& strategy)
    {
        isStop = false;
        LoadData(data);
        std::cout << "xxx tickDataStore size " << tickDataStore.size() << std::endl;
        dataReplayer.reset(new MarketDataReplayer(tickDataStore));
        orderManager.reset(new FakeOrderManager);
        LoadTestStrategy(strategy);

         auto notifyOrder = [&]()
         {
             while(!isStop)
             {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                orderManager->SendNotify();
             }
         };
        orderManagerNotifyThread = std::thread(notifyOrder);

    }

    void TestBed::LoadData(const std::string& dirName)
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
                        tickDataStore.push_back(MarketDataStore(x.path().string()));
                   }
               }
          }
          else
          {
              tickDataStore.push_back(MarketDataStore(dirName));
          }
    }

    void TestBed::LoadTestStrategy(const std::string& dynamicLib)
    {
        auto funptr = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(dynamicLib.c_str(),"createStrategy");
        BluesTrading::IStrategy* strp  = funptr(&logger, &configureManager, dataReplayer.get(), dataReplayer->getTimerProvider(), orderManager.get());
        testStrategy.reset(strp);
    }

    void TestBed::run()
    {
        dataReplayer->subscribeInstrument(1, testStrategy.get());
        dataReplayer->startReplay(20131010,20131011);
     
        isStop = true;
        std::cout << "wait NotifyThread Join "  << std::endl;
        orderManagerNotifyThread.join();
        std::cout << "wait NotifyThread Joined "  << std::endl;
    }

}