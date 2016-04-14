#include "util.h"
#include "testfixture.h"
#include "dynamicloader.h"
#include "datacache.h"

#include "boost/filesystem.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include <fstream>
#include <iostream>
#include <chrono>
#include <exception>
#include <algorithm> 
#include <thread>
#include <atomic>
#include <future>

namespace BluesTrading
{


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

    void TestFixture::Init(TestRequest& request, DataCache* data)
    {
        //  postCount = 0;
        data_ = data;
        datasrc = getDataSrcInfoFromRequest(request);
        singleDataSrcInfo = datasrc[0];

        for (auto& singleData : datasrc)
        {
            std::thread  fetchThread ( [&](){prepareDataCache(singleData);std::cout << "Finish prepareDataCache \n";} );
            remote_IOthread.swap(fetchThread);
        }





        dumpedfileName = dumpDllFile(request);

        strLibrary = new CDynamicLibrary(dumpedfileName);
    

        auto createStrategyFun = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(strLibrary,"createStrategy");

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

             auto startday = getDateFromNum(singleDataSrcInfo.start_date);
             auto preday_of_start_day = startday - boost::gregorian::days(1);
           

            *inst.current_date = getNumFromDate(preday_of_start_day);
           //   std::cout << "StartDay " << startday << " PreDay " << preday_of_start_day << " num :" << *inst.current_date << std::endl;
            allStrInst.push_back(inst);
        }

        std::cout << "create TestInst " << allStrInst.size()  <<std::endl;



        boost::asio::io_service::work* worker = new boost::asio::io_service::work(io_);
        std::thread buildDataReplayer([=](){prepareMarketDataReplayer(); delete worker; std::cout << "Finish all MarketDataReplayer Build\n"; });
        local_IOthread.swap(buildDataReplayer);

        int cores = std::thread::hardware_concurrency();
        for (int i = 0; i != cores * 2; ++i)
        {
            auto workThread = std::make_shared<std::thread>([&](){io_.run();});
            workerThreads.push_back(workThread);
        }
        std::cout << "Create Work Thread Group " << workerThreads.size() <<  std::endl;


    }

    void TestFixture::clean()
    {
        allStrInst.clear();
        delete strLibrary;
        boost::filesystem::remove(dumpedfileName);
    }

    std::string TestFixture::dumpDllFile(TestRequest& request)
    {
        srand(time(nullptr));
        std::string dllFile =  boost::lexical_cast<std::string>(rand()) +  ".dll";
        /*    dllFile +=;*/
        // std::string dllFile = "tempfile.dll";
        std::cout << "Dump Dll File to " << dllFile << std::endl;
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
        ret.timerProvider.reset(new FakeTimerProvider);
        ret.current_date = std::make_shared<uint32_t>(0);
        ret.logger = std::make_shared<nullLogger>();

        BluesTrading::IStrategy* strp  = createFun("teststr",  ret.logger.get(), &configureManager,  &nullDataReplayer,  
            ret.timerProvider.get(),  ret.orderManager.get(), ret.posManager.get());

        if (strp == nullptr)
        {
            std::cout << "create null strategy " << std::endl;
        }

        ret.testStrategy.reset(strp);
        ret.orderManager->setPosMgr(ret.posManager.get());
        ret.timerProvider->registerTimerConsumer(ret.posManager.get());
        return ret;
    }



    void TestFixture::prepareDataCache(DataSrcInfo& singleData)
    {
        data_->addDataCacheRequest(singleData);
    }

  
    void TestFixture::prepareMarketDataReplayer()
    {
        DataSrcInfo& target = singleDataSrcInfo;
        boost::gregorian::date start(getDateFromNum(target.start_date));
        boost::gregorian::date end(getDateFromNum(target.end_date));
        boost::gregorian::days one_day(1);
        auto timestart = std::chrono::high_resolution_clock::now();

        maxLevels = boost::lexical_cast<uint32_t>(target.datasrcInfo[2]);
        dayInMemoryCount = 0;
        for (auto date = start; date < end; date += one_day)
        {
            auto dayofweek = date.day_of_week();
            if (dayofweek == boost::date_time::Sunday || dayofweek == boost::date_time::Saturday)
            {
                continue;
            }

            // sleep 2 milliseconds avoid the async launch not add dayInMemoryCount in time.
            std::this_thread::sleep_for(std::chrono::milliseconds(2));

            while (dayInMemoryCount >= MaxDayInMemory)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            uint32_t dateint = boost::lexical_cast<uint32_t>(boost::gregorian::to_iso_string(date));
            std::async(std::launch::async, &TestFixture::processOneDay, this, dateint);
        }
    }
    

    void TestFixture::addMarketDataStoreToMarketReplay(std::shared_ptr<MarketDataReplayerMultiThread>& replayer, const std::string& instrument, uint32_t date)
    {
        uint32_t date_instrument = getInstrumentIndex(instrument);
        std::string cache_path;
        do
        {
            cache_path = data_->getDataCache(date_instrument, date, maxLevels);
            if (cache_path.empty())
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } while (cache_path.empty());
        replayer->addDataStore(std::move(MarketDataStore(cache_path)));
    }

    void TestFixture::processOneDay(uint32_t date)
    {
        uint32_t maxLevels = boost::lexical_cast<uint32_t>(singleDataSrcInfo.datasrcInfo[2]);
        auto deleteofDataReplayer = [&](MarketDataReplayerMultiThread* p)
        {
            dayInMemoryCount--;
            std::cout << "Free Data For Date " << p->getDate() << "\n";
            delete p;
        };
        ++dayInMemoryCount;
        std::shared_ptr<MarketDataReplayerMultiThread> data(new MarketDataReplayerMultiThread(date), deleteofDataReplayer);
        std::vector<std::future<void>> loadresult;
        for (auto& instrument : datasrc[0].instruments)
        {
            loadresult.emplace_back(
                std::async(std::launch::async, &TestFixture::addMarketDataStoreToMarketReplay, this, std::ref(data), std::ref(instrument), date));
        }

        for (auto& ret: loadresult)
        {
            ret.get();
        }

        for (auto inst : allStrInst)
        {
            std::function<void()> runDonday = std::bind(&TestFixture::runDataOnDay, this, inst, data);
            io_.post(runDonday);
        }

    }


    void TestFixture::run()
    {
        for (auto& worker : workerThreads)
        {
            if(worker->joinable()) worker->join();
        }
        if(remote_IOthread.joinable()) remote_IOthread.join();
        if(local_IOthread.joinable()) local_IOthread.join();
        // std::cout << "PostCount " << postCount << "\n";
    }

    std::pair<std::string, std::vector<std::string>> TestFixture::getResult()
    {
        std::pair<std::string, std::vector<std::string>>  ret;
        for (auto& eachinst : allStrInst)
        {
            auto each_ret = eachinst.logger->getResult();
            for (auto& line : each_ret.second)
            {
                ret.second.emplace_back(std::move(line));
            }
            ret.first.swap(each_ret.first);
        }

        return ret;
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