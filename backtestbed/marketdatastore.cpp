#include "marketdatastore.h"
#include "util.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp> 
#include <string> 
#include <iostream>
#include <cstdio>
#include <array>
#include <memory>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
//#include <cassert>
namespace BluesTrading
{

    std::unique_ptr<CTickData> ExtractTick(const std::string& tickstr, uint32_t instrumentIndex, uint64_t& tot_vol, uint64_t& openinterest, double& trunover)
    {
        std::unique_ptr<CTickData> ret = std::make_unique<CTickData>();
        CTickData& tick = *ret;

        std::vector<std::string> fields;
        boost::split(fields, tickstr, boost::is_any_of(","));

        uint32_t timeInMs = getTime(fields[0]);
        double last_px = atof(fields[1].c_str());
        uint64_t vol_inc = atoi(fields[2].c_str()) * 100;
        double trunover_inc = last_px * vol_inc;

        tot_vol += vol_inc;
        openinterest = 0;
        trunover_inc += trunover_inc ;

        int sell_price_start_index = 4;
        int sell_vol_start_index = 9;
        int buy_price_start_index = 14;
        int buy_vol_start_index = 19;


        auto fillDepths = [&fields, &tick](uint16_t& depths_size, size_t depth_price_start_index, size_t depth_size_start_index)
        {
            for (int i = 0; i != 5; i++)
            {
                uint32_t size = atoi(fields[depth_size_start_index + i].c_str());
                double price = atof(fields[depth_price_start_index +i].c_str());
                if (size > 0)
                {
                    tick.depths.emplace_back(CTickData::Depth{price, size});
                }
                else
                {
                    depths_size = i ;
                    return;
                }
            }
        };


        fillDepths(tick.bidLevels, buy_price_start_index, buy_vol_start_index);
        fillDepths(tick.askLevels, sell_price_start_index, sell_vol_start_index);

        
        tick.instIndex = instrumentIndex;
        tick.timeInMS = timeInMs;
        tick.tot_vol = tot_vol;
        tick.openinterest = 0;

        tick.last_price = last_px;
        tick.turnover = trunover;
        return std::move(ret);
    }

    void MarketDataStore::sort()
    {
        auto sortfunForTick = [](const std::unique_ptr<CTickData>& lr, const std::unique_ptr<CTickData>& rr)
        {
            return lr->timeInMS < rr->timeInMS;
        };
        std::sort(tickDataVec.begin(), tickDataVec.end(), sortfunForTick);
    }

    void MarketDataStore::loadDataFromFile(const std::string& fileName)
    {

        if (fileName.substr(fileName.size() - 3,3) == "csv")
        {
            loadFromRawFile(fileName);
        }
        else
        {
            loadFromBinFile(fileName);
        }
    }

    void MarketDataStore::loadFromRawFile(const std::string& fileName)
    {
        //std::cout << "load tick from " <<fileName << std::endl;
        std::ifstream file(fileName);
        if(!file.good())
        {
            std::cout << "open file fail " <<fileName << std::endl;
            return;
        }
        std::string line; 
        std::getline(file, line);   //first line, header;
        std::getline(file, line);   //second line. first data
        std::string datestr = line.substr(0,10);  //yyyy-mm-dd
        instIndex =  atoi(fileName.substr(fileName.size() - 10,6).c_str());

        date = getDate(datestr);
        uint64_t tot_vol = 0;
        uint64_t openinterest = 0;
        double trunover = 0;

        tickDataVec.emplace_back(ExtractTick(line, instIndex, tot_vol, openinterest, trunover));


        for(std::string line; std::getline(file, line); )
        {
            // std::cout << line << std::endl;
            
            tickDataVec.emplace_back(ExtractTick(line, instIndex, tot_vol, openinterest, trunover));
        }
        file.close();
    }

    void MarketDataStore::loadFromBinFile(const std::string& filename)
    {
        std::ifstream file(filename);

        boost::archive::binary_iarchive ia(file);
        ia  >> *this;

        std::cout << "load from file "<< filename << " Inst:" << this->instIndex << " date:" << this->date << " TickCount:"<< this->tickDataVec.size() << std::endl;
    }

    void MarketDataStore::saveToBinFile(const std::string& filename)
    {
        std::ofstream file(filename);

        boost::archive::binary_oarchive oa(file);
        oa  << *this;
        file.close();
    }


}


