
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
#include <cstdlib>
#include <utility>
#include <memory>
#include <vector>

#include <boost/asio.hpp>
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

TestResult HandleTestRequest(TestRequest request, DataCache* datacache)
{
    TestFixture fixture;
    fixture.Init(request, datacache);
    std::cout << "========  Start Run ====================" << std::endl;
    fixture.run();

    std::vector<std::string> allResult = fixture.getResult();
    TestResult ret;
    std::cout << "=========== End Run getResult ========" << std::endl;
    for (auto& each : allResult)
    {
        ret.add_resultitem(each);
        std::cout << each << "\n";
    }
    std::cout << std::endl;
    fixture.clean();
    return ret;
}

#include <array>
std::vector<DataSrcInfo> getDataSrcInfo(const std::vector< std::map<std::string, std::string> >& config)
{


    const int substr_off = strlen("DataSrc_");

    std::vector<DataSrcInfo> ret;
    DataSrcInfo currentDataSrc;

    for (auto& each_pair : *config.begin())
    {
        const std::string& key_value = each_pair.first;
        const std::string& value = each_pair.second;
        if (key_value.find("DataSrc_") != std::string::npos)
        {
            std::string subkey = key_value.substr(substr_off);
            if (subkey.find("Instruments") != std::string::npos)
            {
                boost::split(currentDataSrc.instruments, value, boost::algorithm::is_any_of("#"));
            }
            else if (subkey.find("StartDate") != std::string::npos)
            {
                currentDataSrc.start_date = atoi(value.c_str());
            }
            else if (subkey.find("EndDate") != std::string::npos)
            {
                currentDataSrc.end_date = atoi(value.c_str());
            }
            else if (subkey.find("Type") != std::string::npos)
            {
                currentDataSrc.datasrcType = atoi(value.c_str());
            }
            else if (subkey.find("Info") != std::string::npos)
            {
                boost::split(currentDataSrc.datasrcInfo, value, boost::algorithm::is_any_of("#"));
            }
        }
    }

    if (currentDataSrc.instruments.size() > 0)
    {
        ret.push_back(currentDataSrc);
    }

    return ret;
}

TestRequest CreateTestRequest(const std::string& dllFile,  const std::string& configFile)
{
    // std::string dllFile = argv[2];
    std::string dllbytes = readFile(dllFile);

    TestRequest request;
    request.set_dllfile(dllbytes.data(), dllbytes.size());

    // = argv[3];
    std::ifstream configFileStream(configFile);
    auto allconfig = parserProps(configFileStream);
    //configFileStream.seekg(0);
    std::vector<DataSrcInfo>  datasrcInfo = getDataSrcInfo(allconfig);

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



using boost::asio::ip::tcp;


size_t read_buf(boost::asio::ip::tcp::socket& s, boost::asio::streambuf& buf)
{
    int size = 0;
    boost::asio::read(s, boost::asio::buffer(&size, sizeof(int)));
    if (size == 0)
    {
        std::cout << "No data...." << std::endl;
        return 0 ;
    }
    return boost::asio::read(s, buf, boost::asio::transfer_exactly(size));
}

size_t write_buf(boost::asio::ip::tcp::socket& s, boost::asio::streambuf& buf)
{
    int size = buf.size();
    boost::asio::write(s, boost::asio::buffer(&size, sizeof(int)));
    return boost::asio::write(s, buf, boost::asio::transfer_exactly(size));
}

template<typename ProtoBufMessage>
size_t recvMessage(boost::asio::ip::tcp::socket& s,ProtoBufMessage& msg)
{
    boost::asio::streambuf buf;
    size_t readsize = read_buf(s ,buf);

    std::istream remotestream(&buf);

    if(! msg.ParseFromIstream(&remotestream))
    {
        std::cout << "Receive the Message  Error"  << std::endl;
    }
    return readsize;
}


template<typename ProtoBufMessage>
size_t sendMessage(boost::asio::ip::tcp::socket& s,ProtoBufMessage& msg)
{
    boost::asio::streambuf outbuf;
    std::ostream ostreambuf(&outbuf);
    msg.SerializeToOstream(&ostreambuf);
    return  write_buf(s, outbuf);
}

void session(tcp::socket sock, DataCache* datacache)
{
    try
    {
        TestRequest request;
        size_t read_size = recvMessage(sock, request);
        std::cout << "Receive buf size "  << read_size << std::endl;

        TestResult ret = HandleTestRequest(request, datacache);

        std::cout << "Finish Run TestRquest \n";

        size_t result_size = sendMessage(sock, ret);

    
 
        std::cout << sock.remote_endpoint().address() <<":" <<  sock.remote_endpoint().port() << "  session complete \n";
        std::cout << "Result size " <<   ret.resultitem_size() << " Send Buff Size " << result_size << "\n";
        sock.close();

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

void server(boost::asio::io_service& io_service, unsigned short port,DataCache* datacache)
{
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        tcp::socket sock(io_service);
        a.accept(sock);
        std::cout << "Client Connect In " << sock.remote_endpoint().address() <<":" <<  sock.remote_endpoint().port() << std::endl;
        std::thread(session, std::move(sock), datacache).detach();
    }
}

int startserver(const std::string& port, DataCache* datacache)
{
    try
    {
        boost::asio::io_service io_service;
        server(io_service, std::atoi(port.c_str()), datacache);
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}

void HandleRequestRemote(const std::string& ip, const std::string& port, TestRequest& request)
{
    boost::asio::io_service io_service;
    tcp::socket sock(io_service);
    tcp::resolver resolver(io_service);
    std::cout << "try to connect to " << ip << ":" << port << std::endl;
    boost::asio::connect(sock, resolver.resolve({ip, port}));

    sendMessage(sock, request);

    TestResult result;
    size_t recv_bytes = recvMessage(sock, result);
    std::cout << "recv " << recv_bytes<< " bytes \n";

    std::cout << "remote result  size " << result.resultitem_size() << "================ \n" ;
    for (auto& result_item: result.resultitem())
    {
        std::cout << result_item << "\n";
    }

}

// get the hardware info. and avgLoad current
double getLocalHostRuningStatus()
{
    auto ret = getCpuStatus();
    static double rttusage = ret;
    rttusage = rttusage * 0.9 + 0.1 * ret;
    return rttusage;
}

void broadcastStatus(const std::string& ip, const std::string port)
{
   std::this_thread::sleep_for(std::chrono::seconds(1));
   
   getLocalHostRuningStatus();
}

void testload(const char* argv)
{
    MarketDataStore inst;
    inst.loadFromBinFile(argv);
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
        else if(cmd == "server")
        {
            startserver( argv[2], &datacache);
        }
        else if(cmd == "client")
        {
            auto request =  CreateTestRequest(argv[2],argv[3]);
            HandleRequestRemote(argv[4], argv[5], request);

        }
        else if(cmd == "backend")
        {
            testBedRun(argv[2],argv[3], argv[4] , argv[5]);
        }
        else if(cmd == "tr")
        {
            auto request =  CreateTestRequest(argv[2], argv[3]);
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
            inst.instruments.push_back(argv[2]);
            inst.start_date = atoi(argv[3]);
            inst.end_date = atoi(argv[4]);
            inst.datasrcInfo.push_back(argv[5]);
            inst.datasrcInfo.push_back(argv[6]);
            inst.datasrcInfo.push_back(argv[7]);
            datacache.addDataCacheRequest(inst);
        }
        //else if (cmd == "testload")
        //{
        //    testload(argv[2]);
        //}
        //else if (cmd == "testload2")
        //{
        //    std::string filename = datacache.getDataCache(5200000,20160201);
        //    std::cout << "DataCache Find Cache File Name " << filename << std::endl;
        //    testload(filename.c_str());
        //}
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Test Catch " << std::endl;
    }

    return 0;
}


