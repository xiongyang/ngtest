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

        void postRunWork(TestInstGroup inst, std::shared_ptr<MarketDataReplayerMultiThread> data)
        {
            auto runDataOnDay = [=]()
            {
                if (*inst.current_date != 0)
                {
                    auto current_end_day = getDateFromNum( *inst.current_date);
                    current_end_day += boost::gregorian::days(1);

                    auto current_data_day = getDateFromNum(data->getDate());

                    if (current_end_day != current_data_day)
                    {
                        // not ready for pre day. re post this work;
                        postRunWork(inst, data);
                        return;
                    }
                }

  

                std::set<ITickDataConsumer*> consumer;
                consumer.insert(inst.testStrategy.get());
                consumer.insert(inst.posManager.get());

                data->StartReplay(consumer, inst.timerProvider.get());
                *inst.current_date = data->getDate();
            };

            io_.post([=](){runDataOnDay();});
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