#pragma once
#include <string>

#include "MarketData.h"
#include "OrderData.h"

#include "ITimer.h"
#include "IOrderManager.h"
#include "ILogger.h"
#include "IConfigureManager.h"
#include "IMarketDataProvider.h"

namespace BluesTrading
{
    class IStrategy :
        public ITimerConsumer, public IOrderDataConsumer, public ITickDataConsumer, public IConfigureable
    {
    public:
        //virtual void onTimer(uint32_t eventID);

    };

    typedef  IStrategy* StrategyFactoryFun(ILogger* , IConfigureManager*, IMarketDataProvider* , ITimerProvider*, IOrderManger*);
    //IStrategy* createStrategy(ILogger* , IConfigureManager*, IMarketDataProvider* , ITimerProvider*, IOrderManger*)
    //class IStrategyFactory
    //{
    //public:
    //	virtual  = 0;
    //};
}


