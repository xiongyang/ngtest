#pragma once

#include "fakeordermanager.h"
#include "faketimerprovider.h"
#include "marketdatareplayer.h"
#include "marketdatastore.h"
#include "IStrategy.h"

#include <memory>
#include <vector>
#include <thread>

namespace BluesTrading
{
    class TestBed
    {
    public:
        void Init();

    private:
        void LoadData();
        void LoadTestStrategy(const std::string& dynamicLib);

    private:
        std::vector<std::shared_ptr<MarketDataStore> > tickDataStore;
        std::shared_ptr<FakeOrderManager> orderManager;
        std::shared_ptr<MarketDataReplayer> dataReplayer;
        std::thread orderManagerNotifyThread;
        std::shared_ptr<IStrategy>  testStrategy;
    };
}