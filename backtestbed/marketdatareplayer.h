#pragma once

#include "IMarketDataProvider.h"
#include "faketimerprovider.h"

#include <map>
#include <vector>
#include <set>

namespace BluesTrading
{
    struct MarketDataStore;
    class FakeTimerProvider;

    class MarketDataReplayer :public IMarketDataProvider
    {
    public:
        MarketDataReplayer(std::vector<MarketDataStore*> datestore);
        virtual void subscribeInstrument(uint32_t instrumentID,ITickDataConsumer* handler ) override;
        virtual void unSubscribeInstrument(uint32_t instrumentID, ITickDataConsumer* handler) override;
        virtual void subscribeAllInstrument(ITickDataConsumer* handler) override;
        virtual void unSubscribeAllInstrument(ITickDataConsumer* handler) override;


    public:
        ITimerProvider* getTimerProvider() {return &timerProvider;}


    public:
        //    [startdate , enddate)  startdate is include but enddate is not
        void startReplay(uint32_t startdate, uint32_t enddate);
    private:
         std::set<ITickDataConsumer*> getAllSubscriber();
    private:
        std::set<ITickDataConsumer*>    alldateSubscriber;
        std::map<uint32_t, std::set<ITickDataConsumer*> > subscribeByInst;
        std::map<uint32_t, std::vector<MarketDataStore*> > dataByDate;
        std::map<uint32_t, std::vector<CTickData<1>*> >   dataByDateSorted;
        
        FakeTimerProvider   timerProvider;
    
    };


}