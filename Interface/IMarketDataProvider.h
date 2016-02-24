#pragma once
#include "MarketData.h"

namespace BluesTrading
{

    class ITickDataConsumer
    {
    public:
        virtual ~ITickDataConsumer() {};

        virtual void onStartDay(uint32_t date) {};
        virtual void onEndDay(uint32_t date) {};

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