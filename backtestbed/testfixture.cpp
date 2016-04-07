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


const int MaxDayInMemory = 10;

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
            std::thread  fetchThread ( [&](){prepareDataCache(singleData);} );
            fetchThread.detach();
        }





        dumpedfileName = dumpDllFile(request);
        auto createStrategyFun = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(dumpedfileName.c_str(),"createStrategy");

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
        std::thread buildDataReplayer([=](){prepareMarketDataReplayer(); delete worker;});
        buildDataReplayer.detach();



        int cores = std::thread::hardware_concurrency();
        //int thread_num  = std::min(cores + 4,  int(cores * 1.5));
        //thread_num = std::min(int(allStrInst.size()), thread_num);
        for (int i = 0; i != cores * 2; ++i)
        {
            auto workThread = std::make_shared<std::thread>([&](){io_.run();});
            workerThreads.push_back(workThread);
        }
        std::cout << "Create Work Thread Group " << workerThreads.size() <<  std::endl;
    }

    void TestFixture::clean()
    {
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

    std::atomic<int>  dayInMemoryCount ;
    void TestFixture::prepareMarketDataReplayer()
    {
        DataSrcInfo& target = singleDataSrcInfo;
        boost::gregorian::date start(getDateFromNum(target.start_date));
        boost::gregorian::date end(getDateFromNum(target.end_date));
        boost::gregorian::days one_day(1);
        auto timestart = std::chrono::high_resolution_clock::now();

        uint32_t maxLevels = boost::lexical_cast<uint32_t>(target.datasrcInfo[2]);
        dayInMemoryCount = 0;
        for (auto date = start; date != end; date += one_day)
        {
            auto dayofweek = date.day_of_week();
            if(dayofweek ==  boost::date_time::Sunday || dayofweek ==   boost::date_time::Saturday)
            {
                continue;
            }
            // waitforDataSlotAviale();
            while (dayInMemoryCount >= MaxDayInMemory)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }


            uint32_t dateint = boost::lexical_cast<uint32_t>( boost::gregorian::to_iso_string(date));
            std::vector<MarketDataStore> alldata;
            for (auto& instrument :    datasrc[0].instruments)
            {
                uint32_t date_instrument = getInstrumentIndex(instrument);
                std::string cache_path;
                do 
                {
                    cache_path =  data_->getDataCache(date_instrument, dateint, maxLevels);
                    if (cache_path.empty())
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        std::cout << "data not ready  Inst:"  << date_instrument << " date:" << dateint << "\n";
                    }
                } while (cache_path.empty());

                alldata.emplace_back(cache_path);   
            }

            {

                std::cout << "Load   date:" << dateint << "  in Memory" <<"\n";
                auto deleteofDataReplayer = [&](MarketDataReplayerMultiThread* p)
                {
                    dayInMemoryCount -- ;
                    std::cout << "Free Data For Date " << p->getDate() << "\n";
                    delete p;
                };

                dayInMemoryCount++;
                std::shared_ptr<MarketDataReplayerMultiThread> data( new MarketDataReplayerMultiThread(alldata, dateint),  deleteofDataReplayer);



                for (auto inst : allStrInst)
                {
                    std::function<void()> runDonday=  std::bind( &TestFixture::runDataOnDay, this, inst, data );
                    io_.post(runDonday);
                }
            }
        }


    }

    //void TestFixture::waitforDataSlotAviale()
    //{
    //    do 
    //    {
    //         cleanFinishedDataReplyer();
    //         if (dateReplayerStored.size() >= MaxDayInMemory)
    //         {
    //              std::this_thread::sleep_for(std::chrono::seconds(1));
    //         }
    //    } while (dateReplayerStored.size() >= MaxDayInMemory);

    //}

    //void TestFixture::cleanFinishedDataReplyer()
    //{
    //    uint32_t min_usage_date = 0;
    //    for (auto& str : allStrInst)
    //    {
    //        if (*str.current_date == 0)
    //        {
    //            //  it not finished any day yet. so no clean
    //            return;
    //        }

    //        if (min_usage_date == 0)
    //        {
    //            min_usage_date = *str.current_date;
    //        }
    //        else
    //        {
    //            min_usage_date = std::min(min_usage_date, *str.current_date);
    //        }
    //    }

    //    std::set<uint32_t> removedays;
    //    for (auto& each : dateReplayerStored)
    //    {
    //        if(each.first <= min_usage_date)
    //            removedays.insert(each.first);
    //    }

    //    {
    //        std::lock_guard<std::shared_timed_mutex>  guard(dateReplayerMutex);
    //        for(auto & each_remove_day : removedays)
    //        {
    //            dateReplayerStored.erase(each_remove_day);
    //        }
    //    }


    //    std::cout << "Remove " << removedays.size() << " Days Date From Memory" << std::endl;
    //}

    //void TestFixture::runForDay(TestInstGroup inst)
    //{

    //   uint32_t targetDate = 0;
    //   if (*inst.current_date == 0)
    //   {
    //       targetDate = datasrc[0].start_date;
    //   }
    //   else
    //   {
    //       auto start_ = getDateFromNum( *inst.current_date);
    //       start_ += boost::gregorian::days(1);

    //       targetDate = getNumFromDate(start_);
    //       
    //   }


    //   if (targetDate >= datasrc[0].end_date)
    //   {
    //       std::cout << "Finished to Date " << targetDate << " endDate:" <<  datasrc[0].end_date << std::endl;
    //       //end run of this inst
    //       return;
    //   }
    //    

    //   std::shared_ptr<MarketDataReplayerMultiThread> data =  getMarketReplayer(targetDate);
    //   if (data)
    //   {
    //       std::cout << "runForDay Start Run Day " << targetDate << "data Date:" << data->getDate()  << " ThreadID :" << std::this_thread::get_id() << "\n";
    //       std::set<ITickDataConsumer*> consumer;
    //       consumer.insert(inst.testStrategy.get());
    //       consumer.insert(inst.posManager.get());

    //       data->StartReplay(consumer, inst.timerProvider.get());
    //        std::cout << "runForDay Finished Run Day " << targetDate << "\n";
    //       *inst.current_date = targetDate;
    //       postRunWork(inst);
    //   }
    //   else
    //   {
    //      //std::cout << "Not get Data for Date Yet." << targetDate << "\n";
    //      //std::this_thread::sleep_for(std::chrono::seconds(1));
    //      postRunWork(inst);
    //   }
    //}

    //std::shared_ptr<MarketDataReplayerMultiThread> TestFixture::getMarketReplayer(uint32_t date)
    //{
    //   std::shared_lock<std::shared_timed_mutex>  shared_lock(dateReplayerMutex);
    //   auto iter =  dateReplayerStored.find(date);
    //   if (iter != dateReplayerStored.end())
    //   {
    //       return iter->second;
    //   }
    //   else
    //   {
    //       return std::shared_ptr<MarketDataReplayerMultiThread>();
    //   }
    //}

    void TestFixture::run()
    {
        for (auto& worker : workerThreads)
        {
            if(worker->joinable()) worker->join();
        }
        // std::cout << "PostCount " << postCount << "\n";
    }

    std::vector<std::string> TestFixture::getResult()
    {
        std::vector<std::string>  ret;
        for (auto& eachinst : allStrInst)
        {
            auto each_ret = eachinst.logger->getResult();
            ret.insert(ret.end(), each_ret.begin(), each_ret.end());
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