#include "marketdatareplayer.h"
#include "marketdatastore.h"

#include <algorithm>
#include <iostream>

namespace BluesTrading 
{

MarketDataReplayer::MarketDataReplayer(std::vector<MarketDataStore> datestore)
{
    std::cout << "create MarketDataReplayer store size:" <<  datestore.size() << std::endl;
    for (std::vector<MarketDataStore>::iterator iter = datestore.begin(); iter != datestore.end(); ++iter)
    {
        uint32_t date = iter->date;
        std::vector<CTickData>& targetDate = dataByDateSorted[date];
        targetDate.insert(targetDate.end(), iter->tickDataVec.begin(),  iter->tickDataVec.end());
    }


    auto sortfunForTick = [](const CTickData& lr , const CTickData& rr)
    {
        return lr.timeInMS < rr.timeInMS;
    };

    for (auto iter = dataByDateSorted.begin(); iter != dataByDateSorted.end(); ++iter)
    {
        std::vector<CTickData>& vec = iter->second;
        std::sort(vec.begin(), vec.end(), sortfunForTick);
       // std::cout << "date:"<< iter->first << " tickcount:" << vec.size();
    }
}

void MarketDataReplayer::subscribeInstrument(uint32_t instrumentID,ITickDataConsumer* handler)
{
    std::cout << "handler:"<< handler << " subscribe inst:" << instrumentID <<  " onDataSrc:"<< this << std::endl;
    subscribeByInst[instrumentID].insert(handler);
}

void MarketDataReplayer::unSubscribeInstrument(uint32_t instrumentID, ITickDataConsumer* handler)
{
    subscribeByInst[instrumentID].erase(handler);
}

void MarketDataReplayer::subscribeAllInstrument(ITickDataConsumer* handler)
{
    allSubscribe_.insert(handler);
}

void MarketDataReplayer::unSubscribeAllInstrument(ITickDataConsumer* handler)
{
    allSubscribe_.erase(handler);
}


void MarketDataReplayer::startReplayAllData()
{
    uint32_t  startdate = dataByDateSorted.begin()->first;
    uint32_t  enddate = dataByDateSorted.rbegin()->first;
    startReplay(startdate, enddate + 1);
}

void MarketDataReplayer::startReplay(uint32_t startdate, uint32_t enddate)
{
    std::cout << "start Date " << startdate  << "  to "  << enddate << std::endl;
    for(auto iter = dataByDateSorted.begin(); iter != dataByDateSorted.end(); ++iter)
    {
        if (iter->first < startdate )
        {
            continue;
        }
        if(iter->first >= enddate)
        {
            break;
        }
        timerProvider.startDate(iter->first);

        std::vector<CTickData>& tickForDay = iter->second;

        for(auto& tick : tickForDay)
        {
            timerProvider.setNextTickTime(tick.timeInMS);
            auto& subscribeVec = subscribeByInst[tick.instIndex];
            for (auto eachsubscribe : subscribeVec)
            {
                eachsubscribe->onMarketData(tick);
            }

            for (auto eachAllSubscribe : allSubscribe_)
            {
                  eachAllSubscribe->onMarketData(tick);
            }
        }

        timerProvider.endDate(iter->first);
    }
}

std::set<ITickDataConsumer*> MarketDataReplayer::getAllSubscriber()
{
    std::set<ITickDataConsumer*> ret;
    for (auto each : subscribeByInst)
    {
        ret.insert(each.second.begin(), each.second.end());
    }

    return ret;
}

MarketDataReplayerMultiThread::MarketDataReplayerMultiThread(std::vector<MarketDataStore> datestore, uint32_t date)
    : date_(date)
{
    std::cout << "create MarketDataReplayerMultiThread store size:" <<  datestore.size() << " date:" <<  date << "\n";


    auto sortfunForTick = [](const CTickData& lr , const CTickData& rr)
    {
        return lr.timeInMS < rr.timeInMS;
    };

    for (std::vector<MarketDataStore>::iterator iter = datestore.begin(); iter != datestore.end(); ++iter)
    {
        std::vector<CTickData> tempret;
        std::merge(allTicks_.begin(), allTicks_.end(), iter->tickDataVec.begin(), iter->tickDataVec.end(), tempret.begin(), sortfunForTick);
        allTicks_.swap(tempret);
    }
}



void MarketDataReplayerMultiThread::StartReplay(std::set<ITickDataConsumer*> consumer, FakeTimerProvider* timerProvider) const
{
    timerProvider->startDate(date_);

    for(auto& tick : allTicks_)
    {
        timerProvider->setNextTickTime(tick.timeInMS);
        for (auto& eachConsumer : consumer)
        {
            eachConsumer->onMarketData(tick);
        }
    }

    timerProvider->endDate(date_);
}

}
