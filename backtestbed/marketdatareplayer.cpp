#include "marketdatareplayer.h"
#include "marketdatastore.h"

#include <algorithm>
#include <iostream>

namespace BluesTrading 
{

MarketDataReplayerMultiThread::MarketDataReplayerMultiThread(uint32_t date)
    : date_(date)
{
   // std::cout << "create MarketDataReplayerMultiThread store size:" <<  datestore.size() << " date:" <<  date << "\n";


    //auto sortfunForTick = [](const CTickData& lr , const CTickData& rr)
    //{
    //    return lr.timeInMS < rr.timeInMS;
    //};

    ////std::vector<CTickData> tempret;
    ////for (std::vector<MarketDataStore>::iterator iter = datestore.begin(); iter != datestore.end(); ++iter)
    ////{
    ////    tempret.clear();
    ////    std::merge(allTicks_.begin(), allTicks_.end(), iter->tickDataVec.begin(), iter->tickDataVec.end(), tempret.begin(), sortfunForTick);
    ////    allTicks_.swap(tempret);
    ////}

    //for (std::vector<MarketDataStore>::iterator iter = datestore.begin(); iter != datestore.end(); ++iter)
    //{
    //    allTicks_.insert(allTicks_.end(), iter->tickDataVec.begin(), iter->tickDataVec.end());
    //    std::sort(allTicks_.begin(), allTicks_.end(), sortfunForTick);
    //    //    std::sort(vec.begin(), vec.end(), sortfunForTick);
    //}

   // std::cout << "1 create MarketDataReplayerMultiThread store size:" <<  datestore.size() << " date:" <<  date << "\n";
}



void MarketDataReplayerMultiThread::StartReplay(std::set<ITickDataConsumer*> consumer, FakeTimerProvider* timerProvider) const
{
    timerProvider->startDate(date_);

    for(auto& tick : allTicks_)
    {
        timerProvider->setNextTickTime(tick->timeInMS);
        for (auto& eachConsumer : consumer)
        {
            eachConsumer->onMarketData(*tick);
        }
    }

    timerProvider->endDate(date_);
}

void MarketDataReplayerMultiThread::addDataStore(MarketDataStore&& datastore)
{    
    std::lock_guard<std::mutex> lk(addmutex);

    auto iter1 = allTicks_.begin();
    auto iter2 = datastore.tickDataVec.begin();
    std::vector<std::unique_ptr<CTickData>> result;
    for (;iter1 != allTicks_.end() && iter2 != datastore.tickDataVec.end();)
    {

        std::unique_ptr<CTickData>& ref1 = *iter1;
        std::unique_ptr<CTickData>& ref2 = *iter2;
        if (ref1->timeInMS < ref2->timeInMS)
        {
            result.emplace_back(std::move(*iter1));
            iter1 ++;
        }
        else
        {
             result.emplace_back(std::move(*iter2));
             iter2 ++;
        }
    }

  
    for (;iter1 != allTicks_.end(); ++iter1)
    {
         result.emplace_back(std::move(*iter1));
    }


    for (;iter2 != datastore.tickDataVec.end(); ++iter2)
    {
        result.emplace_back(std::move(*iter2));
    }

    allTicks_.swap(result);
}

}
