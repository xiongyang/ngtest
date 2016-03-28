#include "util.h"
#include "testfixture.h"
#include "dynamicloader.h"
#include "datacache.h"

#include "boost/filesystem.hpp"

#include <fstream>
#include <iostream>
#include <chrono>
#include <exception>

namespace BluesTrading
{

    void TestFixture::Init(TestRequest& request, DataCache* data)
    {
        data_ = data;

        fetchDataCacheThread = std::thread ( [&](){prepareDataCache(request);} );
 
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
        //bluestrading::istrategy* strp  = createfun("teststr", 
        //    &logger, &configuremanager,  datareplayer.get(), 
        //    datareplayer->gettimerprovider(),  ret.ordermanager.get(),
        //    ret.posmanager.get());

        //if (strp == nullptr)
        //{
        //    std::cout << "create null strategy " << std::endl;
        //}
        //ret.testStrategy.reset(strp);
        //ret.orderManager->setPosMgr(ret.posManager.get());
        //dataReplayer->getTimerProvider()->registerTimerConsumer(ret.posManager.get());
        //dataReplayer->subscribeAllInstrument(ret.posManager.get());
        return ret;
    }


    std::vector<DataSrcInfo> getDataSrcInfoFromRequest(TestRequest& request)
    {
         std::vector<DataSrcInfo> ret;
         for (auto& datasrcinfo :request.datasrc())
         {
             DataSrcInfo inst;
             for(auto& each_id : datasrcinfo.instrument())
             {
                 inst.instruments.push_back(each_id);
             }

             for(auto& each_info : datasrcinfo.datasrcinfo())
             {
                 inst.datasrcInfo.push_back(each_info);
             }


             inst.start_date = datasrcinfo.start_date();
             inst.end_date = datasrcinfo.end_date();
             inst.datasrcType = datasrcinfo.datasrctype();
             ret.push_back(inst);
         }

         return ret;
    }

    void TestFixture::prepareDataCache(TestRequest& request)
    {
        std::cout << "Start Fetch DataCache \n";
        std::vector<DataSrcInfo> src= getDataSrcInfoFromRequest(request);
        for (auto& datasrc : src)
        {
             data_->addDataCacheRequest(datasrc);
        }
       
    }

    void TestFixture::run()
    {
        //dataReplayer->startReplayAllData();

        if(fetchDataCacheThread.joinable())   fetchDataCacheThread.join();
      
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