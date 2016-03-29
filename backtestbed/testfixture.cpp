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

const int MaxDayInMemory = 5;

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
        data_ = data;
        datasrc= getDataSrcInfoFromRequest(request);

        for (auto& singleData : datasrc)
        {
             std::thread  fetchThread ( [&](){prepareDataCache(singleData);} );
             fetchThread.detach();
        }

        std::thread buildDataReplayer([&](){prepareMarketDataReplayer();});
        buildDataReplayer.detach();

 
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

            postRunWork(inst);
        }
        std::cout << "create TestInst " << allStrInst.size()  <<std::endl;


        std::cout << "Create Work Thread Group " << std::endl;
        int cores = std::thread::hardware_concurrency();
        int thread_num  = std::min(cores + 4,  int(cores * 1.5));
        thread_num = std::min(int(allStrInst.size()), thread_num);
        for (int i = 0; i != thread_num; ++i)
        {
            workerThreads.emplace_back([&](){io_.run();});
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
        ret.timerProvider.reset(new FakeTimerProvider);
        ret.current_date = std::make_shared<uint32_t>(0);

        BluesTrading::IStrategy* strp  = createFun("teststr",  &logger, &configureManager,  &nullDataReplayer,  
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

        boost::gregorian::date start(getDateFromNum(datasrc[0].start_date));
        boost::gregorian::date end(getDateFromNum(datasrc[0].end_date));
        boost::gregorian::days one_day(1);

        for (auto date = start; date != end; date += one_day)
        {
            {
                std::lock_guard<std::mutex>  guard(dateReplayerMutex);
                waitforDataSlotAviale();
            }

            uint32_t dateint = boost::lexical_cast<uint32_t>( boost::gregorian::to_iso_string(date));
            std::vector<MarketDataStore> alldata;
            for (auto& instrument :    datasrc[0].instruments)
            {
                uint32_t date_instrument = getInstrumentIndex(instrument);
                std::string cache_path;
                do 
                {
                    cache_path =  data_->getDataCache(date_instrument, dateint);
                    if (cache_path.empty())
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                } while (cache_path.empty());

                alldata.emplace_back(cache_path);   
            }
            std::cout << "Load   date:" << dateint << "  in Memory" << std::endl;
            {
                std::lock_guard<std::mutex>  guard(dateReplayerMutex);
                dateReplayerStored[dateint] = std::make_shared<MarketDataReplayerMultiThread>(alldata, dateint);
            }
        }
   
    }

    void TestFixture::waitforDataSlotAviale()
    {
        do 
        {
             cleanFinishedDataReplyer();
             if (dateReplayerStored.size() >= MaxDayInMemory)
             {
                  std::this_thread::sleep_for(std::chrono::seconds(1));
             }
        } while (dateReplayerStored.size() >= MaxDayInMemory);

    }

    void TestFixture::cleanFinishedDataReplyer()
    {
        uint32_t min_usage_date = 0;
        for (auto& str : allStrInst)
        {
            if (*str.current_date == 0)
            {
                //  it not finished any day yet. so no clean
                return;
            }

            if (min_usage_date == 0)
            {
                min_usage_date = *str.current_date;
            }
            else
            {
                min_usage_date = std::min(min_usage_date, *str.current_date);
            }
        }

        std::set<uint32_t> removedays;
        for (auto& each : dateReplayerStored)
        {
            if(each.first <= min_usage_date)
                removedays.insert(each.first);
        }
        for(auto & each_remove_day : removedays)
        {
            dateReplayerStored.erase(each_remove_day);

        }

        std::cout << "Remove " << removedays.size() << " Days Date From Memory" << std::endl;
    }

    void TestFixture::runForDay(TestInstGroup inst)
    {

       uint32_t targetDate = 0;
       if (*inst.current_date == 0)
       {
           targetDate = datasrc[0].start_date;
       }
       else
       {
           auto start_ = getDateFromNum( *inst.current_date);
           start_ += boost::gregorian::days(1);

           targetDate = getNumFromDate(start_);
           
       }


       if (targetDate >=  datasrc.rbegin()->end_date)
       {
           //end run of this inst
           return;
       }
        

       std::shared_ptr<MarketDataReplayerMultiThread> data =  getMarketReplayer(targetDate);
       if (data)
       {
           std::cout << "runForDay Start Run Day " << targetDate << "data Date:" << data->getDate() << "\n";
           std::set<ITickDataConsumer*> consumer;
           consumer.insert(inst.testStrategy.get());
           consumer.insert(inst.posManager.get());

           data->StartReplay(consumer, inst.timerProvider.get());
            std::cout << "runForDay Finished Run Day " << targetDate << "\n";
           *inst.current_date = targetDate;
           postRunWork(inst);
       }
       else
       {
          std::this_thread::sleep_for(std::chrono::seconds(1));
           postRunWork(inst);
       }
    }

    std::shared_ptr<MarketDataReplayerMultiThread> TestFixture::getMarketReplayer(uint32_t date)
    {
       std::lock_guard<std::mutex>  guard(dateReplayerMutex);
       auto iter =  dateReplayerStored.find(date);
       if (iter != dateReplayerStored.end())
       {
           return iter->second;
       }
       else
       {
           return std::shared_ptr<MarketDataReplayerMultiThread>();
       }
    }

    void TestFixture::run()
    {
        for (auto& worker : workerThreads)
        {
            if(worker.joinable()) worker.join();
        }
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