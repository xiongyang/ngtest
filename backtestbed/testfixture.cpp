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
        auto createStrategyFun = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(filename.c_str(),"createStrategy");

        if(!createStrategyFun)
        {
            std::cout << "Load DLL Fun Fail " << std::endl;
            return ;
        }


        for (auto& configMessage : request.configspace())
        {
            TestInstGroup inst = LoadTestInstGroup(createStrategyFun);
            std::string configstring ;
            configMessage.SerializeToString(&configstring);
            inst.testStrategy->onMessage(configstring);
            allStrInst.push_back(inst);
        }

        std::cout << "create TestInst " << allStrInst.size()  <<std::endl;
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

        boost::filesystem::path file(dllFile);
        permissions(file, boost::filesystem::perms::all_all);

        return dllFile;
    }

    TestFixture::TestInstGroup TestFixture::LoadTestInstGroup(BluesTrading::StrategyFactoryFun createFun)
    {
        TestFixture::TestInstGroup ret;
        ret.orderManager.reset(new FakeOrderManager);
        ret.posManager.reset(new testPositionManger);
        BluesTrading::IStrategy* strp  = createFun("teststr", 
            &logger, &configureManager,  dataReplayer.get(), 
            dataReplayer->getTimerProvider(),  ret.orderManager.get(),
            ret.posManager.get());

        if (strp == nullptr)
        {
            std::cout << "Create Null Strategy " << std::endl;
        }
        ret.testStrategy.reset(strp);
        ret.orderManager->setPosMgr(ret.posManager.get());
        dataReplayer->getTimerProvider()->registerTimerConsumer(ret.posManager.get());
        dataReplayer->subscribeAllInstrument(ret.posManager.get());
        return ret;
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