#include "testbed.h"
#include "dynamicloader.h"

namespace BluesTrading
{

    void TestBed::Init()
    {
        LoadData();
         std::vector<MarketDataStore* > tickDataStoreRef;
         for (auto iter = tickDataStore.begin(); iter != tickDataStore.end(); ++iter)
         {
             tickDataStoreRef.push_back((*iter).get());
         }
        dataReplayer.reset(new MarketDataReplayer(tickDataStoreRef));
        orderManager.reset(new FakeOrderManager);

        

       // tickDataStore->LoadData("test.dat");

    }

    void TestBed::LoadData()
    {
        tickDataStore.push_back(std::make_shared<MarketDataStore>());
    }

    void TestBed::LoadTestStrategy(const std::string& dynamicLib)
    {
        auto funptr = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(dynamicLib.c_str(),"createStrategy");
        //CDynamicLibrary dll;
        //dll.Open(argv[1]);
        //auto funptr = reinterpret_cast<BluesTrading::StrategyFactoryFun*>(dll.GetProc("createStrategy"));
        BluesTrading::IStrategy* strp =  funptr(NULL,NULL,NULL,NULL,NULL);
    }

}