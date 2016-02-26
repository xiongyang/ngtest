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
    };

    typedef  IStrategy* StrategyFactoryFun(const char* name, ILogger* , IConfigureManager*, IMarketDataProvider* , ITimerProvider*, IOrderManger*);

}


