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
    }

    void TestBed::LoadData(const std::string& dirName)
    { 
        auto insertToTickDataStore = [&](const std::string& fileName)
        {
              tickDataStore.push_back(MarketDataStore(fileName));
        };
        traverseDir(dirName, insertToTickDataStore);
    }

    void TestBed::LoadTestStrategy(const std::string& dynamicLib)
    {
        auto funptr = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(dynamicLib.c_str(),"createStrategy");
 
        BluesTrading::IStrategy* strp  = funptr("teststr", 
            &logger, &configureManager,  dataReplayer.get(), dataReplayer->getTimerProvider(), orderManager.get(),
            &posManager);
        testStrategy.reset(strp);  // TODO we should  delelte it from dll so
        orderManager->setPosMgr(&posManager);
        dataReplayer->getTimerProvider()->registerTimerConsumer(&posManager);
        dataReplayer->subscribeAllInstrument(&posManager);

       posManager.setInitCash(10000000.0);
    }

    void TestBed::run(uint32_t startday , uint32_t end_day)
    {
        dataReplayer->startReplay(startday,end_day);
    }

    void TestBed::onMessage(const std::string& propName)
    {

    }

    std::string TestBed::getDisplayMessage()
    {

    }

    std::string TestBed::getConfigurableMessage()
    {

    }

    std::string TestBed::getName()
    {

    }

}