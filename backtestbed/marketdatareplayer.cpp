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
    std::cout << "create MarketDataReplayer store size:" <<  datestore.size() << std::endl;
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
    startReplay(startdate, enddate);
}

void MarketDataReplayer::startReplay(uint32_t startdate, uint32_t enddate)
{
    for(auto iter = dataByDateSorted.begin(); iter != dataByDateSorted.end(); ++iter)
    {
        std::cout << "Date " << iter->first << std::endl;
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

}
