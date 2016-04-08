#pragma once

#include "fakeordermanager.h"
#include "faketimerprovider.h"
#include "marketdatareplayer.h"
#include "marketdatastore.h"
#include "IStrategy.h"
#include "marketdatastore.h"
#include "testpositionmanager.h"
#include "bluemessage.pb.h"
#include "nullLogger.h"
#include "testConfigureManager.h"

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <iostream>

#include <boost/asio/io_service.hpp>


namespace BluesTrading
{
    class      DataCache;

    class TestFixture :public IConfigureable
    {
        struct TestInstGroup
        {
            std::shared_ptr<FakeOrderManager> orderManager;
            std::shared_ptr<testPositionManger>   posManager;
            std::shared_ptr<IStrategy>  testStrategy;
            std::shared_ptr<FakeTimerProvider>  timerProvider;
          //  std::shared_ptr<boost::asio::io_service::strand> strandForThisStrategy;
             std::shared_ptr<uint32_t>  current_date;
             std::shared_ptr<nullLogger>  logger;;
        };
    public:
        void Init(TestRequest& request, DataCache* data);
        void clean();

        uint32_t getNextWorkingDay(uint32_t current_day)
        {
            auto current_end_day = getDateFromNum( current_day);

            auto dayofweek = current_end_day.day_of_week();
            if(dayofweek ==   boost::date_time::Friday )
            {
                current_end_day += boost::gregorian::days(3);
            }
            else if(dayofweek ==   boost::date_time::Saturday)
            {
                current_end_day += boost::gregorian::days(2);
            }
            else
            {
                current_end_day += boost::gregorian::days(1);
            }
            return getNumFromDate(current_end_day);
        }


        void runDataOnDay(TestInstGroup inst,  std::shared_ptr<MarketDataReplayerMultiThread> data)
        {
            uint32_t targetDate = getNextWorkingDay(*inst.current_date);
            if(targetDate == data->getDate())
            {
                std::set<ITickDataConsumer*> consumer;
                consumer.insert(inst.testStrategy.get());
                consumer.insert(inst.posManager.get());
                data->StartReplay(consumer, inst.timerProvider.get());
                *inst.current_date = data->getDate();
            }
            else
            {
                std::function<void()> runDonday=  std::bind( &TestFixture::runDataOnDay, this, inst, data );
                io_.post(runDonday);
            }
        }

        void run();
        std::vector<std::string> getResult();

    private:
        //// dir or file
        //void LoadData(TestRequest& request);
        std::string dumpDllFile(TestRequest& request);
        TestInstGroup LoadTestInstGroup(BluesTrading::StrategyFactoryFun createFun);


        void prepareDataCache(DataSrcInfo& request);
        void prepareMarketDataReplayer();
        //void waitforDataSlotAviale();
        //void cleanFinishedDataReplyer();
        //void runForDay(TestInstGroup inst);
        //std::shared_ptr<MarketDataReplayerMultiThread> getMarketReplayer(uint32_t date);

      
    public:
        virtual void onMessage(const std::string& propName) override;
        virtual std::string getDisplayMessage() override;	
        virtual std::string getConfigurableMessage() override;
        virtual std::string getName() override;

    private:

        std::vector<TestInstGroup>  allStrInst;
        nullLogger  logger;
        TestConfigureManager configureManager;
        DataCache* data_;
        NullMarketDataProvider nullDataReplayer; // useless just for create the strategy

        std::vector<std::shared_ptr<std::thread> >workerThreads;
        boost::asio::io_service io_;

        std::vector<DataSrcInfo> datasrc;
        DataSrcInfo             singleDataSrcInfo;
        //int postCount;
        std::string dumpedfileName;
    };
}