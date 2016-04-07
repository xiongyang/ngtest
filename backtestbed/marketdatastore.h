#pragma once

#include <vector>
#include <map>
#include "MarketData.h"
#include <iostream>

#include <boost/serialization/vector.hpp>

namespace BluesTrading
{
    struct MarketDataStore
    {
        uint32_t instIndex;
        uint32_t date;  //YYYYMMDD
        uint32_t maxLevels; // store the maxLevels
        std::vector<CTickData> tickDataVec;

        MarketDataStore() = default;
        MarketDataStore(uint32_t inst, uint32_t pdate, uint32_t a_maxLevels =  20): instIndex(inst), date(pdate) , maxLevels(a_maxLevels){ }
        explicit MarketDataStore(const std::string& filename)
        {
            loadDataFromFile(filename);
        }

        void sort();
        void loadDataFromFile(const std::string& fileName);
        void loadFromRawFile(const std::string& fileName);
        void loadFromBinFile(const std::string& filename);
        void saveToBinFile(const std::string& filename);
        void loadDataFromDB(const std::string& db, const std::string& query);


        template<class Archive>
        void serialize(Archive & ar, const unsigned int version )
        {
            ar & instIndex;
            ar & date;
            ar & maxLevels;
            ar & tickDataVec;
        }
    };

}