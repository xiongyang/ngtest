#pragma once

#include "IMarketDataProvider.h"
#include "faketimerprovider.h"
#include "marketdatastore.h"

#include <map>
#include <vector>
#include <set>
#include <unordered_map> 

namespace BluesTrading
{
    struct MarketDataStore;
    class FakeTimerProvider;

    class MarketDataReplayer :public IMarketDataProvider
    {
    public:
        explicit MarketDataReplayer(std::vector<MarketDataStore> datestore);
        virtual void subscribeInstrument(uint32_t instrumentID,ITickDataConsumer* handler ) override;
        virtual void unSubscribeInstrument(uint32_t instrumentID, ITickDataConsumer* handler) override;
        virtual void subscribeAllInstrument(ITickDataConsumer* handler) override;
        virtual void unSubscribeAllInstrument(ITickDataConsumer* handler) override;


    public:
        ITimerProvider* getTimerProvider() {return &timerProvider;}


    public:
        //    [startdate , enddate)  startdate is include but enddate is not
        void startReplayAllData();
        void startReplay(uint32_t startdate, uint32_t enddate);
    private:
         std::set<ITickDataConsumer*> getAllSubscriber();
    private:
        std::set<ITickDataConsumer*>    alldateSubscriber;
        std::unordered_map<uint32_t, std::set<ITickDataConsumer*> > subscribeByInst;
        std::map<uint32_t, std::vector<CTickData> >   dataByDateSorted;
        std::set<ITickDataConsumer*>    allSubscribe_;
        FakeTimerProvider   timerProvider;
    };

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
        MarketDataReplayerMultiThread(std::vector<MarketDataStore> datestore, uint32_t date);
        virtual void subscribeInstrument(uint32_t instrumentID,ITickDataConsumer* handler ) override {};
        virtual void unSubscribeInstrument(uint32_t instrumentID, ITickDataConsumer* handler) override {};
        virtual void subscribeAllInstrument(ITickDataConsumer* handler) override {};
        virtual void unSubscribeAllInstrument(ITickDataConsumer* handler) override {};

    public:
        void StartReplay(std::set<ITickDataConsumer*>  consumer, FakeTimerProvider* timerProvider) const;
        uint32_t getDate() {return date_;}
    private:
        uint32_t date_;
        std::vector<CTickData> allTicks_;
    };


}