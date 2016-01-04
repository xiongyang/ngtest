#include "marketdatareplayer.h"
#include "marketdatastore.h"

#include <algorithm>
#include <iostream>

namespace BluesTrading 
{


bool sortfunForTick(const CTickData<1>* lr , const CTickData<1>* rr)
{
    return lr->timeInMS < rr->timeInMS;
}

template<typename T>
void insertTicks(std::vector<CTickData<1>*>& vecForAllDepths, const std::vector<T*>& vec)
{
    for (auto iter = vec.begin(); iter != vec.end(); ++iter)
    {
        vecForAllDepths.push_back(reinterpret_cast<CTickData<1>*>(*iter));
    }
}


MarketDataReplayer::MarketDataReplayer(std::vector<MarketDataStore*> datestore)
{
    for (std::vector<MarketDataStore*>::iterator iter = datestore.begin(); iter != datestore.end(); ++iter)
    {
        uint32_t date = (*iter)->date;
        dataByDate[date].push_back(*iter);
        insertTicks(dataByDateSorted[date], (*iter)->getStore<1>());
        insertTicks(dataByDateSorted[date], (*iter)->getStore<5>());
        insertTicks(dataByDateSorted[date], (*iter)->getStore<10>());
        insertTicks(dataByDateSorted[date], (*iter)->getStore<20>());
    }

    for (std::map<uint32_t, std::vector<CTickData<1>*> >::iterator iter = dataByDateSorted.begin(); 
        iter != dataByDateSorted.end(); ++iter)
    {
        std::vector<CTickData<1>*>& vec = iter->second;
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

//void MarketDataReplayer::setTimer(ITimerConsumer* consumer, uint32_t eventID, uint32_t timeInMS, bool repeat)
//{
//
//}
//
//std::uint32_t MarketDataReplayer::getCurrentTimeMsInDay()
//{
//
//}
//
//std::uint32_t MarketDataReplayer::getCurrentDate()
//{
//
//}


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

      //  std::cout << "Start Tick Data Replay date " << iter->first << std::endl;

        std::vector<CTickData<1>*>& tickForDay = iter->second;

        for(auto tick : tickForDay)
        {
            auto& subscribeVec = subscribeByInst[tick->instIndex];
            for (auto eachsubscribe : subscribeVec)
            {
                DispatchMarketDataByLevels(*tick, eachsubscribe);
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

    return std::move(ret);
}

}
