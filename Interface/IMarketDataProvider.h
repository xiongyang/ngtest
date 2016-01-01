#include "MarketData.h"

namespace BluesTrading
{

    class ITickDataConsumer
    {
    public:
        virtual ~ITickDataConsumer() {};

        virtual void onStartDay(uint32_t date) {};
        virtual void onEndDay(uint32_t date) {};

        virtual void onMarketData(const TickDataLevel1&) {};
        virtual void onMarketData(const TickDataLevel5&)  {};
        virtual void onMarketData(const TickDataLevel10&) {};
        virtual void onMarketData(const TickDataLevel20&) {};
        virtual void onOtherLevelsMarketData(const CTickData<1>& tick) {};		// for any other Levels.

       
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

    static void DispatchMarketDataByLevels(const CTickData<1>& tick, ITickDataConsumer* handle)
    {
        switch (tick.depthsNum)
        {
        case 1:
            handle->onMarketData(*(TickDataLevel1*)&tick);
            break;
        case 5:
            handle->onMarketData(*(TickDataLevel5*)&tick);
            break;
        case 10:
            handle->onMarketData(*(TickDataLevel10*)&tick);
            break;
        case 20:
            handle->onMarketData(*(TickDataLevel20*)&tick);
            break;
        default:
            handle->onOtherLevelsMarketData(tick);
            break;
        }
    }

}