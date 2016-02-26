#pragma once
#include "MarketData.h"

namespace BluesTrading
{

    class ITickDataConsumer
    {
    public:
        virtual ~ITickDataConsumer() {};
        virtual void onMarketData(const CTickData& tick) {} ;       
    };

    class IMarketDataProvider
    {
    public:
        //virtual void setSubscriber(ITickDataConsumer* handler) = 0;
        virtual void subscribeInstrument(uint32_t instrumentID,ITickDataConsumer* handler ) = 0;
        virtual void unSubscribeInstrument(uint32_t instrumentID, ITickDataConsumer* handler) = 0;
        virtual void subscribeAllInstrument(ITickDataConsumer* handler) = 0;
        virtual void unSubscribeAllInstrument(ITickDataConsumer* handler) = 0;
    };

}