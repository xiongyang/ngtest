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
        std::vector<CTickData> tickDataVec;

        MarketDataStore() = default;
        MarketDataStore(uint32_t inst, uint32_t pdate): instIndex(inst), date(pdate) {}
        explicit MarketDataStore(const std::string& filename)
        {
            loadDataFromFile(filename);
        }


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
            ar & tickDataVec;
        }
    };

    uint32_t getDate(const std::string& date);
    uint32_t getTime(const std::string& date_time_str);

    //static std::istream& operator>>(std::istream& istream, MarketDataStore& ref)
    //{
    //    istream.readref.instIndex;
    //    istream >>  ref.date;
    //     int count = 0;
    //    while(istream)
    //    {
    //        count ++;
    //        CTickData data;
    //        istream >> data;
    //        ref.tickDataVec.push_back(std::move(data));
    //    }
    //    std::cout << "read " << count << " ticks" << std::endl;
    //    return istream;
    //}

    //static std::ostream& operator<<(std::ostream& of, const MarketDataStore& ref )
    //{
    //    of <<  ref.instIndex;
    //    of <<  ref.date;
    //    int count = 0; 
    //    for(auto& each :  ref.tickDataVec)
    //    {
    //        count ++;
    //        of << each;
    //    }

    //     std::cout << "save " << count << " ticks" << std::endl;
    //    return of;
    //}


}