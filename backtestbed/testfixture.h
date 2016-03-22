#pragma once

#include "fakeordermanager.h"
#include "faketimerprovider.h"
#include "marketdatareplayer.h"
#include "marketdatastore.h"
#include "IStrategy.h"

#include "nullLogger.h"
#include "testConfigureManager.h"

#include <memory>
#include <vector>
#include <thread>
#include "marketdatastore.h"
#include "testpositionmanager.h"
#include "bluemessage.pb.h"

namespace BluesTrading
{
    class TestFixture :public IConfigureable
    {
        struct TestInstGroup
        {
            std::shared_ptr<FakeOrderManager> orderManager;
            std::shared_ptr<testPositionManger>   posManager;
            std::shared_ptr<IStrategy>  testStrategy;
        };

    public:
        void Init(TestRequest& request);
        void run();
        std::vector<std::string> getResult() {return logger.getResult();}

    private:
        //// dir or file
        void LoadData(TestRequest& request);
        std::string dumpDllFile(TestRequest& request);
        TestInstGroup LoadTestInstGroup(BluesTrading::StrategyFactoryFun createFun);
       
      
    public:
        virtual void onMessage(const std::string& propName) override;
        virtual std::string getDisplayMessage() override;	
        virtual std::string getConfigurableMessage() override;
        virtual std::string getName() override;

    private:
      ////  bool    isStop;
        std::vector<MarketDataStore> tickDataStore;
        std::shared_ptr<MarketDataReplayer> dataReplayer;


        std::vector<TestInstGroup>  allStrInst;

        nullLogger  logger;
        TestConfigureManager configureManager;
    };
}