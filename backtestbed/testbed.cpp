#include <fstream>
#include <iostream>
#include "boost/filesystem.hpp"

#include "testbed.h"
#include "dynamicloader.h"
#include <chrono>
#include "util.h"

namespace BluesTrading
{

    void TestBed::Init(const std::string& data , const std::string& strategy)
    {
     //   isStop = false;
        LoadData(data);
        dataReplayer.reset(new MarketDataReplayer(tickDataStore));
        orderManager.reset(new FakeOrderManager);
        LoadTestStrategy(strategy);

        // auto notifyOrder = [&]()
        // {
        //     while(!isStop)
        //     {
        //       
        //          orderManager->SendNotify();
        //        auto allorders = orderManager->getAllOrders();
        //       
        //        for (OrderDataDetail* each : allorders)
        //        {
        //            if(each->sse_order.orderStatus != SSE_OrderDetail::SSE_OrderTraded)
        //            {
        //                orderManager->MakeOrderTrade(each->orderID);
        //            }
        //        } 
        //     }
        // };
        //orderManagerNotifyThread = std::thread(notifyOrder);

    }

    void TestBed::LoadData(const std::string& dirName)
    { 
        auto insertToTickDataStore = [&](const std::string& fileName)
        {
              tickDataStore.push_back(MarketDataStore(fileName));
        };
        traverseDir(dirName, insertToTickDataStore);
          //if (!boost::filesystem::exists(dirName))
          //{
          //    std::cout << "dir not exists" << dirName;
          //    return;
          //}

          //if (boost::filesystem::is_directory(dirName))
          //{
          //     for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(dirName))
          //     {
          //         if (boost::filesystem::is_regular_file(x))
          //         {
          //              tickDataStore.push_back(MarketDataStore(x.path().string()));
          //         }
          //     }
          //}
          //else
          //{
          //    tickDataStore.push_back(MarketDataStore(dirName));
          //}
    }

    void TestBed::LoadTestStrategy(const std::string& dynamicLib)
    {
        auto funptr = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(dynamicLib.c_str(),"createStrategy");
        //std::cout << "yyyy arguments logger:" << &logger 
        //    << "  configure:" <<  &configureManager 
        //    << "  data:"<< dataReplayer.get()
        //    << " Timer:" << dataReplayer->getTimerProvider() 
        //    << " Order " <<  orderManager.get() << std::endl;
        BluesTrading::IStrategy* strp  = funptr("teststr", 
            &logger, &configureManager,  dataReplayer.get(), dataReplayer->getTimerProvider(), orderManager.get(),
            &posManager);
        testStrategy.reset(strp);  // TODO we should  delelte it from dll so
    }

    void TestBed::run(uint32_t startday , uint32_t end_day)
    {
        dataReplayer->startReplay(startday,end_day);
        //isStop = true;
        //std::cout << "wait NotifyThread Join "  << std::endl;
        //orderManagerNotifyThread.join();
        //std::cout << "wait NotifyThread Joined "  << std::endl;
    }

}