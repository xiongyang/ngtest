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

namespace BluesTrading
{
    class TestBed
    {
    public:
        void Init(const std::string& data , const std::string& strategy);

    public:
        // dir or file
        void LoadData(const std::string& dir_or_file_Name);
        void LoadTestStrategy(const std::string& dynamicLib);
        void run();

    private:
        bool    isStop;
        std::vector<MarketDataStore> tickDataStore;
        std::shared_ptr<FakeOrderManager> orderManager;
        std::shared_ptr<MarketDataReplayer> dataReplayer;
        std::thread orderManagerNotifyThread;
        std::shared_ptr<IStrategy>  testStrategy;
        nullLogger  logger;
        TestConfigureManager configureManager;
    };
}