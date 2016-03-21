#include <fstream>
#include <iostream>
#include "boost/filesystem.hpp"

#include "testfixture.h"
#include "dynamicloader.h"
#include <chrono>
#include "util.h"
#include <exception>

namespace BluesTrading
{

    void TestFixture::Init(TestRequest& request)
    {

        LoadData(request);
        dataReplayer.reset(new MarketDataReplayer(tickDataStore));

        std::string filename = dumpDllFile(request);

        for (auto& each_config : request.configspace())
        {
            const StrategyConfig& configMessage = each_config;
            TestInstGroup inst = LoadTestInstGroup(filename);
            std::string configstring ;
            configMessage.SerializeToString(&configstring);
            inst.testStrategy->onMessage(configstring);

            // all the cash and position init is done in the onMessage

            allStrInst.push_back(inst);
        }

    }

    void TestFixture::LoadData(TestRequest& request)
    { 
        auto insertToTickDataStore = [&](const std::string& fileName)
        {
              tickDataStore.push_back(MarketDataStore(fileName));
        };

        for (auto& eachdatesrc : request.datasrc())
        {
             traverseDir(eachdatesrc, insertToTickDataStore);
        }
       
    }

    std::string TestFixture::dumpDllFile(TestRequest& request)
    {
        std::string dllFile = "tempfile.dll";
        std::fstream dllfilestream(dllFile, std::ios_base::out | std::ios_base::binary);
        if (! dllfilestream)
        {
            std::cerr << " Create TempDll File Fail " << std::endl;
            throw std::exception(/*" Create TempDll File Fail "*/);
        }
        dllfilestream.write(request.dllfile().c_str(), request.dllfile().size());
        dllfilestream.close();
        return dllFile;
    }

    TestFixture::TestInstGroup TestFixture::LoadTestInstGroup(const std::string& dynamicLib)
    {
         TestFixture::TestInstGroup ret;

        auto funptr = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(dynamicLib.c_str(),"createStrategy");
        //if (!funptr)
        //{
        //    std::cerr << "Load Dll Fail " << std::endl;
        //    throw std::exception(/*"Load Dll Fail "*/);
        //}
        ret.orderManager.reset(new FakeOrderManager);
        ret.posManager.reset(new testPositionManger);
        BluesTrading::IStrategy* strp  = funptr("teststr", 
            &logger, &configureManager,  dataReplayer.get(), 
            dataReplayer->getTimerProvider(),  ret.orderManager.get(),
            ret.posManager.get());
        ret.testStrategy.reset(strp);
        ret.orderManager->setPosMgr(ret.posManager.get());

        dataReplayer->getTimerProvider()->registerTimerConsumer(ret.posManager.get());
        dataReplayer->subscribeAllInstrument(ret.posManager.get());

     /*  posManager.setInitCash(10000000.0);*/
    }

    void TestFixture::run()
    {
        dataReplayer->startReplayAllData();
    }

    void TestFixture::onMessage(const std::string& propName)
    {

    }

    std::string TestFixture::getDisplayMessage()
    {

    }

    std::string TestFixture::getConfigurableMessage()
    {

    }

    std::string TestFixture::getName()
    {

    }

}