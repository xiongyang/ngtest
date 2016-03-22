
#include "testbed.h"
#include "marketdatastore.h"
#include "util.h"

#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include "bluemessage.pb.h"
#include "testfixture.h"


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

void HandleTestRequest(TestRequest request)
{
    TestFixture fixture;
    fixture.Init(request);
    std::cout << "============================" <<std::endl;
    fixture.run();

    std::vector<std::string> allResult = fixture.getResult();

    std::cout <<"============================="<<std::endl;
    for (auto& each: allResult)
    {
        std::cout << each << "\n";
    }
    std::cout << std::endl;
}

//void testdumpfile(int argc, char**argv)
//{
//    std::string dllFile = argv[2];
//    std::string dllBytes = readFile(dllFile);
//
//    std::string dumpfile = "tempfile2.dll";
//    std::fstream dumpfile_stream(dumpfile, std::ios_base::out | std::ios_base::binary);
//    if (! dumpfile_stream)
//    {
//        std::cerr << " Create TempDll File Fail " << std::endl;
//        throw std::exception(/*" Create TempDll File Fail "*/);
//    }
//    dumpfile_stream.write(dllBytes.c_str(), dllBytes.size());
//    dumpfile_stream.close();
//}



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


    HandleTestRequest(request);
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
        try
        {
                    HandleTestRequest(argc, argv);
        }
        catch (std::exception& ex)
        {
        	std::cout << ex.what() << std::endl;
        }

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
    //else if (cmd == "dumpfile")
    //{

    //    testdumpfile(argc, argv);
    //}

    return 0;
}


