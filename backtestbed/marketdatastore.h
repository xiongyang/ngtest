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
        //std::vector<TickDataLevel1*>  level1Data;
        //std::vector<TickDataLevel5*>  level5Data;  
        //std::vector<TickDataLevel10*>  level10Data;  
        //std::vector<TickDataLevel20*>  level20Data;

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

        //template<>
        //std::vector<CTickData<1>*>& MarketDataStore::getStore<1>();
        //template<>
        //std::vector<CTickData<5>*>& MarketDataStore::getStore<5>();
        //template<>
        //std::vector<CTickData<10>*>& MarketDataStore::getStore<10>();
        //template<>
        //std::vector<CTickData<20>*>& MarketDataStore::getStore<20>();
   
        //template<int N>
        //std::vector<CTickData<N>*>& getStore()
        //{
        //    //defalut imple
        //    auto iter = levelDatas.find(N);
        //    if(iter != levelDatas.end())
        //    {
        //        return (* (std::vector<CTickData<N>*>*)iter->second);
        //    }
        //    else
        //    {
        //        levelDatas.insert(new  std::vector<TickDataLevel1*>)
        //    }
        //    return std::vector<TickDataLevel1*>
        //}


    };


    //template<>
    //std::vector<CTickData<1>*>& MarketDataStore::getStore<1>()
    //{
    //    return level1Data;
    //}
    //template<>
    //std::vector<CTickData<5>*>&  MarketDataStore::getStore<5>()
    //{
    //    return level5Data;
    //}
    //template<>
    //std::vector<CTickData<10>*>&  MarketDataStore::getStore<10>()
    //{
    //    return level10Data;
    //}
    //template<>
    //std::vector<CTickData<20>*>&  MarketDataStore::getStore<20>()
    //{
    //    return level20Data;
    //}
}