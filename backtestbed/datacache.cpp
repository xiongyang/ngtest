#include "datacache.h"
#include "MarketData.h"
#include "marketdatastore.h"

#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include <mutex>

#define OTL_STL
#define OTL_ANSI_CPP
#define OTL_ODBC // Compile OTL 4/ODBC
#include "otlv4.h" // include the OTL 4 header file

//#define DEBUG_LOG

namespace BluesTrading
{
    // first level is instrument
    // filename is instrument_date.bin

    void DataCache::InitDataCache(const std::string& dirName)
    {
        localCache_path = dirName;
        if (!boost::filesystem::exists(dirName))
        {
            std::cout << "DataCache dir not exists" << dirName << "\n";
            boost::filesystem::create_directory(dirName);
            return;
        }

        //index_file_path = (dirName);
        //index_file_path.append(LocalCacheFileIndexFileName);
        //if (!boost::filesystem::exists(dirName))
        //{
        //      std::cout << "DataCache IndexFile not exists" << dirName;
        //      return;
        //}


        if (boost::filesystem::is_directory(dirName))
        {
            for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(dirName))
            {

                if (boost::filesystem::is_directory(x))
                {
                    std::string instrumentname = x.path().filename().string();
                    uint32_t instIndex = boost::lexical_cast<uint32_t>(instrumentname);
                    if (instIndex > 10000)
                    {
                        CheckCacheForInst(x.path());
                    }
                }
            }
        }
        else if (boost::filesystem::is_regular_file(dirName))
        {
            std::cout << "DataCache dir not exists" << dirName << "\n";
            return;
        }
    }



    void DataCache::addDataCacheRequest(const DataSrcInfo& datarequest)
    {
        boost::gregorian::date start = getDateFromNum(datarequest.start_date);
        boost::gregorian::date end = getDateFromNum(datarequest.end_date); 
        boost::gregorian::days one_day(1);

        for (auto date = start; date != end; date += one_day)
        {
            uint32_t dateint = getNumFromDate(date);
            getDataFromRemote(datarequest.instruments, datarequest.datasrcInfo, datarequest.datasrcType, dateint);
        }
    }


    std::string DataCache::getDataCache(uint32_t inst, uint32_t date, uint32_t maxLevels)
    {
        auto iter = cache_status[inst].cacheStatus_.find(date);
        if (iter != cache_status[inst].cacheStatus_.end())
        {
           auto& levels_path_map = iter->second;
           auto iter_2 = levels_path_map.find(maxLevels);
           if (iter_2 != levels_path_map.end())
           {
               return iter_2->second;
           }
        }

        return std::string();
    }


    void DataCache::getDataFromRemote(const std::vector<std::string>& instruments, const std::vector<std::string>& dateinfos, uint32_t datatype, uint32_t date)
    {
        switch (datatype)
        {
        case 1:
            getDataFromMSSql(instruments, dateinfos, date);
        default:
            break;
        }
    }


    std::once_flag init_db_flag;
    void initDbSystem()
    {
        otl_connect::otl_initialize();
    }


    void DataCache::getDataFromMSSql(const std::vector<std::string>& instruments, const std::vector<std::string>& dateinfos, uint32_t date)
    {
        try
        {
            std::call_once(init_db_flag, initDbSystem);
            const std::string& mssql_dsn = dateinfos.at(0);
            const std::string& mssql_table = dateinfos.at(1);
            uint32_t max_levels = 10;
            if (dateinfos.size() > 2)
            {
                max_levels = atoi(dateinfos.at(2).c_str());
            }
            otl_connect db;
            std::string loginstring = "DSN=" + mssql_dsn;
            db.rlogon(loginstring.c_str());
#ifdef DEBUG_LOG
            std::cout << "Login to " << loginstring << std::endl;
#endif
            for (auto& inst : instruments)
            {
                uint32_t inst_index = getInstrumentIndex(inst);
                std::string cache_result = getDataCache(inst_index, date, max_levels);

                otl_datetime startTime;
                startTime.year = date / 10000;
                startTime.month = (date - startTime.year * 10000) / 100;
                startTime.day = date % 100;
                startTime.hour = 0;
                startTime.minute = 0;
                startTime.second = 0;
                // startTime.set_non_null();


                otl_datetime endTime;
                endTime.year = date / 10000;
                endTime.month = (date - endTime.year * 10000) / 100;
                endTime.day = date % 100;
                endTime.hour = 23;
                endTime.minute = 59;
                endTime.second = 59;

                if (cache_result.empty())
                {
                    std::string cache_file_name = getInstumentDataPath(inst_index, date, max_levels);
               
           
                    MarketDataStore storeforday(inst_index, date, max_levels);

                    CTickData tick;
                    otl_datetime fulltime;
                    fulltime.frac_precision = 3;

     
                   // endTime.set_non_null();
                    const std::string depthQuery_1 = "bidprice,bidsize,askprice,asksize";
                    const std::string depthQuery_5 =    "bidprice1,bidsize1,bidprice2,bidsize2,bidprice3,bidsize3,bidprice4,bidsize4,bidprice5,bidsize5,"
                        "askprice1,asksize1,askprice2,asksize2,askprice3,asksize3,askprice4,asksize4,askprice5,asksize5";
                    const std::string depthQuery_10 =   "bidprice1,bidsize1,bidprice2,bidsize2,bidprice3,bidsize3,bidprice4,bidsize4,bidprice5,bidsize5,bidprice6,bidsize6,bidprice7,bidsize7,bidprice8,bidsize8,bidprice9,bidsize9,bidprice10,bidsize10,"
                        "askprice1,asksize1,askprice2,asksize2,askprice3,asksize3,askprice4,asksize4,askprice5,asksize5,askprice6,asksize6,askprice7,asksize7,askprice8,asksize8,askprice9,asksize9,askprice10,asksize10";
                    const std::string depthQuery_20 =   "bidprice1,bidsize1,bidprice2,bidsize2,bidprice3,bidsize3,bidprice4,bidsize4,bidprice5,bidsize5,bidprice6,bidsize6,bidprice7,bidsize7,bidprice8,bidsize8,bidprice9,bidsize9,bidprice10,bidsize10,"
                        "bidprice11,bidsize11,bidprice12,bidsize12,bidprice13,bidsize13,bidprice14,bidsize14,bidprice15,bidsize15,bidprice16,bidsize16,bidprice17,bidsize17,bidprice18,bidsize18,bidprice19,bidsize19,bidprice20,bidsize20,"
                        "askprice1,asksize1,askprice2,asksize2,askprice3,asksize3,askprice4,asksize4,askprice5,asksize5,askprice6,asksize6,askprice7,asksize7,askprice8,asksize8,askprice9,asksize9,askprice10,asksize10,"
                        "askprice11,asksize11,askprice12,asksize12,askprice13,asksize13,askprice14,asksize14,askprice15,asksize15,askprice16,asksize16,askprice17,asksize17,askprice18,asksize18,askprice19,asksize19,askprice20,asksize20";

                    std::string query_depth_string;
                    switch(max_levels)
                    {
                    case 1:
                        query_depth_string = depthQuery_1;
                        break;
                    case 5:
                        query_depth_string  = depthQuery_5;
                        break;
                    case 10:
                        query_depth_string  = depthQuery_10;
                        break;
                    case 20:
                        query_depth_string  = depthQuery_20;
                        break;
                    }
                    
                    tick.instIndex = inst_index;
                    std::string query_str = "select fulltime,totalvol,openinterest,tradeprice,turnover," + query_depth_string +   " from " + mssql_table;
                    query_str += " where instrument = '";
                    query_str += inst;
                    query_str += "' and fulltime >:startTime<timestamp>  and fulltime <:endTime<timestamp> ";
                    otl_stream query_stream(50000, // buffer size
                        query_str.c_str(),
                        // SELECT statement
                        db // connect object
                        );
                    query_stream << startTime << endTime;

#ifdef DEBUG_LOG
                    std::cout << "Query: " << query_str << std::endl;
#endif

                    while (!query_stream.eof())
                    {
                        tick.depths.clear();
                        query_stream >> fulltime >> tick.tot_vol >> tick.openinterest >> tick.last_price >> tick.turnover;
                        tick.timeInMS = fulltime.hour * 3600 * 1000 + fulltime.minute * 60 * 1000 + fulltime.second * 1000 + fulltime.fraction;

                        int valid_biddepth = 0;
                        for (int i = 0; i != max_levels; ++i)
                        {
                            CTickData::Depth depth;
                

                            query_stream >> depth.price >> depth.size;
                            if (depth.size > 0)
                            {
                                tick.depths.push_back(depth);
                                valid_biddepth++;
                            }
                        }

                        tick.bidLevels = valid_biddepth;

                        int valid_askdepth = 0;
                        for (int i = 0; i != max_levels; ++i)
                        {
                            CTickData::Depth depth;


                            query_stream >> depth.price >> depth.size;
                            if (depth.size > 0)
                            {
                                tick.depths.push_back(depth);
                                valid_askdepth++;
                            }
                        }

                        tick.askLevels = valid_askdepth;

                        storeforday.tickDataVec.push_back(tick);
                    }

                    storeforday.sort();
                    storeforday.saveToBinFile(cache_file_name);
                   // boost::filesystem::file_size(cache_file_name);
                    cache_status[inst_index].cacheStatus_[date][max_levels] = cache_file_name;
                    std::cout << "Load Data  " << inst << "  Date:" << date << " MaxLevels:" << max_levels <<  " SaveTo " << cache_file_name << std::endl;
                }
            }

            db.logoff();
        }
        catch (otl_exception& p) { // intercept OTL exceptions
            using namespace std;
            cerr << p.msg << endl; // print out error message
            cerr << p.stm_text << endl; // print out SQL that caused the error
            cerr << p.sqlstate << endl; // print out SQLSTATE message
            cerr << p.var_info << endl; // print out the variable that caused the error
        }
      
    }

    void DataCache::CheckCacheForInst(const boost::filesystem::path& checkpath)
    {
        for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(checkpath))
        {
            if (boost::filesystem::is_regular_file(x) && x.path().extension().string() == ".bin")
            {
                std::string fileName = x.path().filename().stem().string();
                std::vector<std::string>  filename_parts;
                boost::split(filename_parts, fileName, boost::is_any_of("_"));
                uint32_t date = boost::lexical_cast<uint32_t>(filename_parts[1]);
                uint32_t inst = boost::lexical_cast<uint32_t>(filename_parts[0]);
                uint32_t maxLevels = boost::lexical_cast<uint32_t>(filename_parts[2]);

                cache_status[inst].cacheStatus_[date][maxLevels] = x.path().string();
                std::cout <<  "LoadCache File for Inst " << inst << " date" << date << " MaxLevel:" << maxLevels << " File:" << x.path().string() << "\n";
            }
            else
            {
                std::cout << "ignore invalid cache fiel " << x.path().string();
            }
        }
    }

    std::string DataCache::getInstrumentDir(uint32_t inst)
    {
        boost::filesystem::path localPath(localCache_path);
        localPath.append( boost::lexical_cast<std::string>(inst));
        return localPath.string();
    }


    std::string DataCache::getInstumentDataPath(uint32_t inst, uint32_t date, uint32_t maxLevels)
    {
        boost::filesystem::path instrumentdir(getInstrumentDir(inst));
        if (!boost::filesystem::exists(instrumentdir))
        {
            boost::filesystem::create_directory(instrumentdir);
        }
        std::string cache_file_name = boost::lexical_cast<std::string>(inst);
        cache_file_name += "_";
        cache_file_name += boost::lexical_cast<std::string>(date);

        cache_file_name += "_";
        cache_file_name += boost::lexical_cast<std::string>(maxLevels);
        cache_file_name += ".bin";

        instrumentdir.append(cache_file_name);
        return instrumentdir.string();
    }

}