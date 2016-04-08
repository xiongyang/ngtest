#pragma once

#include <vector>
#include <map>
#include "MarketData.h"
#include <iostream>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/unique_ptr.hpp>

namespace BluesTrading
{
    struct MarketDataStore
    {
        uint32_t instIndex;
        uint32_t date;  //YYYYMMDD
        uint32_t maxLevels; // store the maxLevels
        std::vector<std::unique_ptr<CTickData>> tickDataVec;

        MarketDataStore() = default;
        MarketDataStore(uint32_t inst, uint32_t pdate, uint32_t a_maxLevels =  20): instIndex(inst), date(pdate) , maxLevels(a_maxLevels){ }
        explicit MarketDataStore(const std::string& filename)
        {
            isNeedUpdateStore = false;
            loadDataFromFile(filename);

            if(isNeedUpdateStore) saveToBinFile(filename);
        }

        void sort();
        void loadDataFromFile(const std::string& fileName);
        void loadFromRawFile(const std::string& fileName);
        void loadFromBinFile(const std::string& filename);
        void saveToBinFile(const std::string& filename);


        //template<class Archive>
        //void serialize(Archive & ar, const unsigned int version )
        //{
        //    ar & instIndex;
        //    ar & date;
        //    ar & maxLevels;
        //    ar & tickDataVec;
        //}

        template<class Archive>
        void save(Archive & ar, const unsigned int version) const
        {
            ar & instIndex;
            ar & date;
            ar & maxLevels;
            size_t size = tickDataVec.size();
            ar & size;
            for (auto& up : tickDataVec)
            {
                ar & *up;
            }
        }
        template<class Archive>
        void load(Archive & ar, const unsigned int version)
        {
            if (version == 0)
            {
                   std::vector<CTickData> oldVector;
                    ar & instIndex;
                    ar & date;
                    ar & maxLevels;
                    ar & oldVector;

                    for (auto& newTick : oldVector)
                    {
                        auto tick = std::make_unique<CTickData>() ;
                        *tick = newTick;
                        tickDataVec.emplace_back(std::move(tick));
                    }
                    isNeedUpdateStore = true;
            }
            else if (version == 1)
            {
                ar & instIndex;
                ar & date;
                ar & maxLevels;
                size_t ticksize = 0;
                ar & ticksize;
                for (int i = 0 ; i != ticksize; ++i)
                {
                    auto tick = std::make_unique<CTickData>() ;
                    ar & *tick;
                    tickDataVec.emplace_back(std::move(tick));
                }
            }
 
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()

        bool isNeedUpdateStore;
    };



}
    // first version not the pt
BOOST_CLASS_VERSION(BluesTrading::MarketDataStore, 1)