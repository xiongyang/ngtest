#include "marketdatareplayer.h"
#include "marketdatastore.h"

#include <algorithm>
#include <iostream>

namespace BluesTrading 
{



//template<typename T>
//void insertTicks(std::vector<CTickData<1>*>& vecForAllDepths, const std::vector<T*>& vec)
//{
//    for (auto iter = vec.begin(); iter != vec.end(); ++iter)
//    {
//        vecForAllDepths.push_back(reinterpret_cast<CTickData<1>*>(*iter));
//    }
//}


MarketDataReplayer::MarketDataReplayer(std::vector<MarketDataStore> datestore)
{
    for (std::vector<MarketDataStore>::iterator iter = datestore.begin(); iter != datestore.end(); ++iter)
    {
        uint32_t date = iter->date;
        std::vector<CTickData>& targetDate = dataByDateSorted[date];
        targetDate.insert(targetDate.end(), iter->tickDataVec.begin(),  iter->tickDataVec.end());
        //insertTicks(dataByDateSorted[date], (*iter)->getStore<1>());
        //insertTicks(dataByDateSorted[date], (*iter)->getStore<5>());
        //insertTicks(dataByDateSorted[date], (*iter)->getStore<10>());
        //insertTicks(dataByDateSorted[date], (*iter)->getStore<20>());
    }


    auto sortfunForTick = [](const CTickData& lr , const CTickData& rr)
    {
        return lr.timeInMS < rr.timeInMS;
    };

    for (auto iter = dataByDateSorted.begin(); iter != dataByDateSorted.end(); ++iter)
    {
        std::vector<CTickData>& vec = iter->second;
        std::sort(vec.begin(), vec.end(), sortfunForTick);
    }
}

void MarketDataReplayer::subscribeInstrument(uint32_t instrumentID,ITickDataConsumer* handler)
{
    subscribeByInst[instrumentID].insert(handler);
}

void MarketDataReplayer::unSubscribeInstrument(uint32_t instrumentID, ITickDataConsumer* handler)
{
    subscribeByInst[instrumentID].erase(handler);
}

void MarketDataReplayer::subscribeAllInstrument(ITickDataConsumer* handler)
{
    //TODO
}

void MarketDataReplayer::unSubscribeAllInstrument(ITickDataConsumer* handler)
{
    //TODO
}


void MarketDataReplayer::startReplay(uint32_t startdate, uint32_t enddate)
{
    for(auto iter = dataByDateSorted.begin(); iter != dataByDateSorted.end(); ++iter)
    {

        std::set<ITickDataConsumer*>  allsubscriber = getAllSubscriber();

        if (iter->first < startdate )
        {
            continue;
        }
        if(iter->first >= enddate)
        {
            break;
        }

        for (auto each : allsubscriber)
        {
            each->onStartDay(iter->first);
        }

        timerProvider.setDate(iter->first);

      //  std::cout << "Start Tick Data Replay date " << iter->first << std::endl;

        std::vector<CTickData>& tickForDay = iter->second;

        for(auto tick : tickForDay)
        {
            timerProvider.setNextTickTime(tick.timeInMS);
            auto& subscribeVec = subscribeByInst[tick.instIndex];
            for (auto eachsubscribe : subscribeVec)
            {
                eachsubscribe->onMarketData(tick);
            }   
        }

        for (auto each : allsubscriber)
        {
            each->onEndDay(iter->first);
        }
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

}
