#include "testbed.h"
#include "dynamicloader.h"

namespace BluesTrading
{

    void TestBed::Init()
    {
        LoadData();
        //std::vector<MarketDataStore> tickDataStoreRef;
        //for (auto iter = tickDataStore.begin(); iter != tickDataStore.end(); ++iter)
        //{
        //    tickDataStoreRef.push_back((*iter).get());
        //}
        dataReplayer.reset(new MarketDataReplayer(tickDataStore));
        orderManager.reset(new FakeOrderManager);

        //LoadTestStrategy("test.dll");

        //tickDataStore->LoadData("test.dat");

    }

    void TestBed::LoadData()
    {       
       // tickDataStore.push_back(std::make_shared<MarketDataStore>());
    }

    void TestBed::LoadTestStrategy(const std::string& dynamicLib)
    {
        auto funptr = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(dynamicLib.c_str(),"createStrategy");
        BluesTrading::IStrategy* strp  = funptr(&logger, &configureManager, dataReplayer.get(), dataReplayer->getTimerProvider(), orderManager.get());
    }

}