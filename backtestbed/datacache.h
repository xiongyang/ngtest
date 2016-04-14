#pragma once

#include "util.h"

#include <string>
#include <unordered_map>
#include <map>

class otl_connect;

//TODO log every cache filesize for valid the Cache file
namespace BluesTrading
{
    const static char * LocalCacheFileIndexFileName = "index.txt";
    class DataCache
    {
    public:
        void InitDataCache(const std::string& datachaceDir);    // check local cache and 
        void addDataCacheRequest(const DataSrcInfo& datarequest);
        void addDataCacheRquetMSSql(const DataSrcInfo& datarequest);

        //get the DataCached File path. if not cached. return empty string
        std::string getDataCache(uint32_t inst, uint32_t date, uint32_t maxLevels);
       

    private:
        void getDataFromRemote(const std::vector<std::string>& instruments, const std::vector<std::string>& dateinfos, uint32_t datatype, uint32_t date);
        void getDataFromMSSql(const std::vector<std::string>& instruments, const std::vector<std::string>& dateinfos, uint32_t date, otl_connect* db, uint32_t max_levels, const std::string& sqlTable);
        void CheckCacheForInst(const boost::filesystem::path& checkpath);
      
        std::string getInstrumentDir(uint32_t inst);
        std::string getInstumentDataPath(uint32_t inst, uint32_t date, uint32_t maxLevels);
        struct InstCacheInfo 
        {
            // key is date
            // value -> key is maxLevels
            std::map<uint32_t, std::map<uint32_t,std::string> > cacheStatus_;      // Every Date Cache Status
        };

        std::unordered_map<uint32_t, InstCacheInfo>  cache_status;
        std::string localCache_path;
        boost::filesystem::path index_file_path;
        std::unordered_map<std::string, uint32_t>  cached_file_size;
    };


}