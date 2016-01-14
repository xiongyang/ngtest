#pragma once

#include <vector>

#include "MarketData.h"

namespace BluesTrading
{
    struct MarketDataStore
    {
        uint32_t instIndex;
        uint32_t date;  //YYYYMMDD


        typedef std::vector<TickDataLevel1*> tickdateVec;       // for poly levels
        std::map<int, tickdateVec*> levelDatas;


        template<int N>
        std::vector<CTickData<N>*>& getStore()
        {
                auto iter = levelDatas.find(N);
                if(iter != levelDatas.end())
                {
                    return (* (std::vector<CTickData<N>*>*)iter->second);
                }
                else
                {
                    iter = levelDatas.insert(std::make_pair(N, new tickdateVec)).first;
                    return (* (std::vector<CTickData<N>*>*)iter->second);
                }
        }


    };
}