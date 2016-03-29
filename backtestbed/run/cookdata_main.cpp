
#include "testbed.h"
#include "marketdatastore.h"
#include "util.h"
#include "bluemessage.pb.h"
#include "testfixture.h"
#include "datacache.h"

#include <iostream>
#include <string>
#include <fstream>
#include <thread>

#include <boost/algorithm/string.hpp>

using namespace BluesTrading;

void cookData(const std::string& dirName)
{

    auto cookDataOne = [](const std::string& filename)
    {
        if(".csv" ==  filename.substr(filename.size()-3 , 4))
        {
            MarketDataStore inst(filename);
            boost::filesystem::path newpath(filename);
            inst.saveToBinFile( newpath.replace_extension(".bin").string());
        }
    };
    traverseDir(dirName, cookDataOne);
}

void testBedRun(const std::string& dir, const std::string& strategy, const std::string& startday, const std::string& endDay)
{
    TestBed inst;
    inst.Init(dir, strategy);
    inst.run(atoi(startday.c_str()), atoi(endDay.c_str()));
    std::cout << std::endl;
    return ;
}

void HandleTestRequest(TestRequest request, DataCache* datacache)
{
    TestFixture fixture;
    fixture.Init(request, datacache);
    std::cout << "========  Start Run ====================" << std::endl;
    fixture.run();

    std::vector<std::string> allResult = fixture.getResult();

    std::cout << "=========== End Run getResult ========" << std::endl;
    for (auto& each : allResult)
    {
        std::cout << each << "\n";
    }
    std::cout << std::endl;
}


std::vector<DataSrcInfo> getDataSrcInfo( const std::unordered_map<std::string, std::string>&  config)
{
    const int substr_off = strlen("DataSrc_");

    std::vector<DataSrcInfo> ret;
    DataSrcInfo currentDataSrc;

    for (auto& each_pair : config)
    {
        const std::string& key_value = each_pair.first;
        const std::string& value = each_pair.second;
        if (key_value.find("DataSrc_") != std::string::npos)
        {
            std::string subkey = key_value.substr(substr_off);
            if(subkey.find("Instruments") != std::string::npos)
            {
                if (currentDataSrc.instruments.size() != 0)
                {
                    ret.push_back(currentDataSrc);
                }
                currentDataSrc.clear();
                boost::split(currentDataSrc.instruments,value, boost::algorithm::is_any_of("#"));
            }
            else if(subkey.find("StartDate") != std::string::npos)
            {
                currentDataSrc.start_date = atoi(value.c_str());
            }
            else if(subkey.find("EndDate") != std::string::npos)
            {
                currentDataSrc.end_date = atoi(value.c_str());
            }
            else if(subkey.find("Type") != std::string::npos)
            {
                currentDataSrc.datasrcType = atoi(value.c_str());
            }
            else if(subkey.find("Info") != std::string::npos)
            {
                boost::split(currentDataSrc.datasrcInfo,value, boost::algorithm::is_any_of("#"));
            }
        }
    }

    if (currentDataSrc.instruments.size() > 0 )
    {
        ret.push_back(currentDataSrc);
    }

    return ret;
}


TestRequest CreateTestRequest(int argc, char**argv)
{
    std::string dllFile = argv[2];
    std::string dllbytes = readFile(dllFile);

    TestRequest request;
    request.set_dllfile(dllbytes.data(), dllbytes.size());

    std::string configFile = argv[3];
    std::ifstream configFileStream(configFile);
    std::vector< std::unordered_map<std::string, std::string> > allconfig = parserProps(configFileStream);

     std::vector<DataSrcInfo>  datasrcInfo = getDataSrcInfo(*allconfig.begin());

     for (auto& eachDataSrc: datasrcInfo)
     {
         DataSrc* datasrcMsg = request.add_datasrc();

         for (auto& each : eachDataSrc.instruments)
         {
             datasrcMsg->add_instrument(each);
         }
         for (auto& each : eachDataSrc.datasrcInfo)
         {
             datasrcMsg->add_datasrcinfo(each);
         }

         datasrcMsg->set_start_date(eachDataSrc.start_date);
         datasrcMsg->set_end_date(eachDataSrc.end_date);
         datasrcMsg->set_datasrctype(eachDataSrc.datasrcType);
     }

    for (auto& each_config : allconfig)
    {
        StrategyConfig* configMessasge = request.add_configspace();
        for (auto& each_pair : each_config)
        {
            if (each_pair.first.find("DataSrc_") == std::string::npos)
            {
                prop* inst =  configMessasge->add_props();
                inst->set_propname(each_pair.first);
                inst->set_value(each_pair.second); 
            }
        }
    }


    std::cout << "dllfile " << dllFile << " Size "<< dllbytes.size() << "\n";
    std::cout << "datasrc size " << request.datasrc_size() << "\n";
    std::cout << "configspace size " << request.configspace_size() << "\n";


    return request;
}

// get the hardware info. and avgLoad current
void getLocalHostRuningStatus()
{
    auto ret = getCpuStatus();
    static double rttusage = ret;
    rttusage = rttusage * 0.9 + 0.1 * ret;
    std::cout << "Cpu Usage " << rttusage << "\n";
}

int main(int argc, char** argv)
{

    DataCache datacache;
    datacache.InitDataCache("LocalCache");
    try
    {
        std::string cmd = argv[1];

        if(cmd == "cook")
        {
            cookData(argv[2]);
        }
        else if(cmd == "backend")
        {
            testBedRun(argv[2],argv[3], argv[4] , argv[5]);
        }
        else if(cmd == "tr")
        {
           auto request =  CreateTestRequest(argc, argv);
           HandleTestRequest(request, &datacache);
        }
        else if (cmd == "usage")
        {
            while(true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                getLocalHostRuningStatus();
            }
        }
        else if (cmd == "bo")
        {
            std::cout << "Send Hello Bo" << std::endl;
            UdpSender sender(argv[2], argv[3]);
            std::cout << "Send Hello to" <<argv[2] << ":" << argv[3] <<  std::endl;
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "Send Hello " << std::endl;
                std::cout << "SendSize " << sender.send("Hello") << std::endl ;
            }
        }
        else if (cmd == "recv")
        {
            UdpReceiver recver(argv[2], argv[3]);
            std::cout << "Send RecvFrom " <<argv[2] << ":" << argv[3] <<  std::endl;
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::string buf;
                recver.receiver(&buf);
                std::cout << "recv  " << buf << std::endl ;
            }
        }
        else if (cmd == "sql")
        {
            DataSrcInfo inst;
            inst.datasrcType = 1;
            inst.instruments.push_back("ag");
            inst.start_date = 20160201;
            inst.end_date = 20160310;
            inst.datasrcInfo.push_back("dl_level2");
            inst.datasrcInfo.push_back("lfull_sunrain_shfe_test");
            datacache.addDataCacheRequest(inst);
            //for (auto& datasrc : request.datasrc())
            //{
            // 

            //}
           
        }
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Test Catch " << std::endl;
    }
    //else if (cmd == "dumpfile")
    //{

    //    testdumpfile(argc, argv);
    //}

    return 0;
}


