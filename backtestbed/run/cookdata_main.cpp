
#include "testbed.h"
#include "marketdatastore.h"
#include "util.h"

#include <iostream>
#include <string>
#include <fstream>
#include "testRequest.pb.h"


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
    return ;
}

void HandleTestRequest(int argc, char**argv)
{
    std::string dllFile = argv[2];
    std::string dllbytes = readFile(dllFile);
    TestRequest request;
    request.set_dllfile(dllbytes.data(), dllbytes.size());

    std::string configFile = argv[3];
    std::ifstream configFileStream(configFile);
    std::vector< std::unordered_map<std::string, std::string> > allconfig = parserProps(configFileStream);


    for (auto& each_config : allconfig)
    {
        StrategyConfig* configMessasge = request.add_configspace();
        for (auto& each_pair : each_config)
        {
              prop* inst =  configMessasge->add_props();
              inst->set_propname(each_pair.first);
              inst->set_value(each_pair.second); 
        }
    }

    for (int i = 4; i !=argc; ++i)
    {
        request.add_datasrc(argv[i]);
    }

    std::cout << "dllfile " << dllFile << " Size "<< dllbytes.size() << "\n";
    std::cout << "datasrc size " << request.datasrc_size() << "\n";
    std::cout << "configspce size " << request.configspace_size() << "\n";

    //UdpSender sender("127.0.0.1 39223");
    //std::string sendbuf;
    //bool ret = request.SerializeToString(&sendbuf);
    //if(!ret)
    //{
    //    std::cout << "TestRequest SerializeToString Fail \n";
    //}
    //sender.send(sendbuf);

  /*  boost::asio::*/
}

// get the hardware info. and avgLoad current
void getLocalHostRuningStatus()
{
    auto ret = getCpuStatus();
    static double rttusage = ret;
    rttusage = rttusage * 0.9 + 0.1 * rttusage;
    std::cout << "Cpu Usage " << rttusage << "\n";
}

int main(int argc, char** argv)
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
    else if(cmd == "request")
    {
        HandleTestRequest(argc, argv);
    }
    else if (cmd == "usage")
    {
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            getLocalHostRuningStatus();
        }
    }

    return 0;
}


