#pragma once

#include "IMarketDataProvider.h"
#include "faketimerprovider.h"
#include "marketdatastore.h"

#include <map>
#include <vector>
#include <set>
#include <unordered_map> 
#include <mutex>

namespace BluesTrading
{
    struct MarketDataStore;
    class FakeTimerProvider;

    class NullMarketDataProvider : public IMarketDataProvider
    {
    public:
        virtual void subscribeInstrument(uint32_t instrumentID,ITickDataConsumer* handler ) override {};
        virtual void unSubscribeInstrument(uint32_t instrumentID, ITickDataConsumer* handler) override {};
        virtual void subscribeAllInstrument(ITickDataConsumer* handler) override {};
        virtual void unSubscribeAllInstrument(ITickDataConsumer* handler) override {};
    };

    // only for one day
    class MarketDataReplayerMultiThread : public IMarketDataProvider
    {
    public:
        //MarketDataReplayerMultiThread(){};
        //virtual ~MarketDataReplayerMultiThread(){}
        explicit MarketDataReplayerMultiThread(uint32_t date);
        virtual void subscribeInstrument(uint32_t instrumentID,ITickDataConsumer* handler ) override {};
        virtual void unSubscribeInstrument(uint32_t instrumentID, ITickDataConsumer* handler) override {};
        virtual void subscribeAllInstrument(ITickDataConsumer* handler) override {};
        virtual void unSubscribeAllInstrument(ITickDataConsumer* handler) override {};

    public:
        void StartReplay(std::set<ITickDataConsumer*>  consumer, FakeTimerProvider* timerProvider) const;
        uint32_t getDate() {return date_;}

        void addDataStore(MarketDataStore&& datastore);
    private:
        uint32_t date_;
        std::vector<std::unique_ptr<CTickData>> allTicks_;
        std::mutex addmutex;
    };


}