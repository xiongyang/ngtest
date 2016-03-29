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
        };
    public:
        void Init(TestRequest& request, DataCache* data);

        void postRunWork(TestInstGroup inst)
        {
           io_.post([=](){runForDay(inst);});
        }

        void run();
        std::vector<std::string> getResult() {return logger.getResult();}

    private:
        //// dir or file
        //void LoadData(TestRequest& request);
        std::string dumpDllFile(TestRequest& request);
        TestInstGroup LoadTestInstGroup(BluesTrading::StrategyFactoryFun createFun);


        void prepareDataCache(DataSrcInfo& request);
        void prepareMarketDataReplayer();
        void waitforDataSlotAviale();
        void cleanFinishedDataReplyer();
        void runForDay(TestInstGroup inst);
        std::shared_ptr<MarketDataReplayerMultiThread> getMarketReplayer(uint32_t date);

      
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

        std::vector<std::thread> workerThreads;
        boost::asio::io_service io_;

        std::vector<DataSrcInfo> datasrc;
        DataSrcInfo             singleDataSrcInfo;
        std::map<uint32_t, std::shared_ptr<MarketDataReplayerMultiThread> > dateReplayerStored;
        std::mutex dateReplayerMutex;
    };
}