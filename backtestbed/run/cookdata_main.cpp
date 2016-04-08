
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
#include <future>

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


TestResult HandleTestRequest(TestRequest request, DataCache* datacache)
{
    TestFixture fixture;
    fixture.Init(request, datacache);
    std::cout << "========  Start Run ====================" << std::endl;
    fixture.run();

    auto allResult = fixture.getResult();
    TestResult ret;
    std::cout << "=========== End Run getResult ========" << std::endl;
    for (auto& each : allResult.second)
    {
        ret.add_resultitem(each);
        std::cout << each << "\n";
    }
    ret.set_headline(allResult.first);
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


// get the hardware info. and avgLoad current
double getLocalHostRuningStatus()
{
    
    //auto ret = getCpuStatus();
    //static double rttusage = ret;
    //rttusage = rttusage * 0.9 + 0.1 * ret;
    //return rttusage;
    return getCpuStatus();
}


using boost::asio::ip::tcp;
using boost::asio::ip::udp;

size_t read_buf(tcp::socket& s, boost::asio::streambuf& buf)
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

size_t write_buf(tcp::socket& s, boost::asio::streambuf& buf)
{
    int size = buf.size();
    boost::asio::write(s, boost::asio::buffer(&size, sizeof(int)));
    return boost::asio::write(s, buf, boost::asio::transfer_exactly(size));
}

size_t read_buf(udp::socket& s, boost::asio::streambuf& buf)
{
    auto max_upd_trans_buff = buf.prepare(1500);
    auto size =  s.receive(max_upd_trans_buff);
    buf.commit(size);
    return size;
}

size_t write_buf(udp::socket& s, boost::asio::streambuf& buf)
{
    return  s.send(boost::asio::buffer(buf.data(), buf.size()));
}


template<typename SockType, typename ProtoBufMessage>
size_t recvMessage(SockType& s,ProtoBufMessage& msg)
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


template<typename SockType, typename ProtoBufMessage>
size_t sendMessage(SockType& s,ProtoBufMessage& msg)
{
    boost::asio::streambuf outbuf;
    std::ostream ostreambuf(&outbuf);
    msg.SerializeToOstream(&ostreambuf);
    return  write_buf(s, outbuf);
}

void broadcastStatus(const std::string& ip, const std::string& port, bool& stop)
{
    NodeStatus msg;
    msg.set_address(ip);
    msg.set_port(port);
    msg.set_cores(std::thread::hardware_concurrency());

    boost::asio::io_service io_service;
    udp::socket sock(io_service, udp::endpoint(udp::v4(),10000));
    //udp::resolver resolver(io_service);
    //auto ep =  resolver.resolve({broadcast_ip, broadcast_port});
    //boost::asio::connect(sock,ep);

    boost::asio::socket_base::broadcast option(true);
    sock.set_option(option);
    udp::endpoint destination(boost::asio::ip::address::from_string("255.255.255.255"), 5000);

    sock.connect(destination);
    // sock.send(boost::asio::buffer(buf, 10));

    while (!stop)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        double usage = getLocalHostRuningStatus();
        msg.set_usage(usage);
        sendMessage(sock, msg);
    }
}



void recvbroadcastStatus(std::map<std::string, NodeStatus>& statusmap, bool& stop)
{
    boost::asio::io_service io_service;
    udp::socket sock(io_service, udp::endpoint(udp::v4(), 5000));
    // udp::socket sock(io_service);
    boost::asio::socket_base::broadcast option(true);
    sock.set_option(option);
    //  socket.receive_from(boost::asio::buffer(buf), sender_endpoint);
    // udp::endpoint destination(boost::asio::ip::address::from_string("255.255.255.255"), 5000);

    // udp::endpoint destination;
    //sock.connect(destination);


    while(!stop)
    {
        //recvbuf.resize(size);
        //std::cout << "recv"
        NodeStatus msg;
        /*    msg.par*/
        auto size = recvMessage(sock, msg);
        statusmap[ msg.address()] = msg;
        std::cout << "RecvMessage " << msg.address() << " : " << msg.port() << "   CpuUsage "<< msg.usage() << " Cores " << msg.cores() << "\n";
    }
}


void session(tcp::socket sock, DataCache* datacache)
{
    try
    {
        TestRequest request;
        size_t read_size = recvMessage(sock, request);
        std::cout << "Receive buf size "  << read_size <<"\n";

        // TestResult ret;
        //for (int i = 0 ; i != 1000; ++i)
        //{
        //    ret = HandleTestRequest(request, datacache);
        //}
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
        //std::thread(session, std::move(sock), datacache).detach();
        session(std::move(sock), datacache);
    }
}

int startserver(const std::string& ip, const std::string& port, DataCache* datacache)
{
    bool stop = false;
    std::thread broadthread(broadcastStatus, ip, port, std::ref(stop));

    try
    {
        boost::asio::io_service io_service;
        server(io_service, std::atoi(port.c_str()), datacache);
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n" << std::endl;
        stop = true;
        broadthread.join();
    }

    return 0;
}

TestRequest splitRequest(TestRequest& request, uint32_t num)
{
    TestRequest ret(request);
    if (num >= request.configspace_size())
    {
        request.clear_configspace();
        return ret;
    }
    else
    {
        ret.clear_configspace();
        for (int i = 0; i != num ; ++i)
        {
            StrategyConfig* configMessasge = ret.add_configspace();
            configMessasge->CopyFrom ((*request.configspace().rbegin()));
            request.mutable_configspace()->RemoveLast();
        }
        return ret;
    } 
}

void mergerResult(TestResult& left, TestResult& right)
{
    left.set_headline(right.headline());
    for (auto& item : right.resultitem())
    {
        left.add_resultitem(item);
    }
}

TestResult HandleRequestRemote(const std::string& ip, const std::string& port, TestRequest& request);
TestResult asyncwork( std::set< NodeStatus*>&  working_node, NodeStatus* p_work_node, TestRequest req) 
{
    std::cout << "Send Work " << req.configspace_size() << " To " << p_work_node->address() << std::endl;
    TestResult nodeResult = HandleRequestRemote(p_work_node->address(), p_work_node->port(), req);
    working_node.erase(p_work_node);
    std::cout << "Finish Work " << req.configspace_size() << " From " << p_work_node->address() << std::endl;
    return nodeResult;
};

TestResult HandleRequestNetwrok(TestRequest& request, std::map<std::string, NodeStatus>& status, double task_factor)
{
    std::this_thread::sleep_for(std::chrono::seconds(3));  // first wait 3 second for receive the network info

    std::set< NodeStatus*> allready_working_node;
    NodeStatus* pNextNode = nullptr;


    std::vector<std::future<TestResult>> resultfutures;

    while (request.configspace_size() > 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        double powerIndex  = 0;
        for (auto& statusItme : status)
        {
            if (allready_working_node.find(&statusItme.second) != allready_working_node.end())
            {
                // This Node Already get work form me
                continue;
            }

            double current_powerIndex =  statusItme.second.cores() * statusItme.second.usage()  /  100.0;
            if (current_powerIndex > powerIndex)
            {
                powerIndex = current_powerIndex;
                pNextNode =   &statusItme.second;
            }
        }

        if(pNextNode)
        {
            uint32_t request_cores = pNextNode->cores() * task_factor;
            TestRequest newrequest = splitRequest(request, request_cores);
            allready_working_node.insert(pNextNode);
            std::cout << "Dispatch " << std::endl;
            auto future_result = std::async(std::launch::async ,asyncwork, std::ref(allready_working_node), pNextNode, newrequest);
            //auto future_result =  std::async(asyncwor2k, stdallready_working_node, newrequest);
            resultfutures.push_back(std::move(future_result));
        }

        std::this_thread::sleep_for(std::chrono::std::chrono::milliseconds (100));
    }


    TestResult result;
    for (auto& futurework : resultfutures)
    { 
        TestResult thisresult= futurework.get();
        mergerResult(result, thisresult);
    }

    return result;
}


TestResult HandleRequestRemote(const std::string& ip, const std::string& port, TestRequest& request)
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
    return result;
}



void testload(const char* argv)
{
    MarketDataStore inst;
    inst.loadFromBinFile(argv);
}

void dumpResult(TestResult& result, const std::string& dumpfile)
{
    std::ofstream file(dumpfile);
    file << result.headline() << "\n";
    for (auto& eachLine: result.resultitem())
    {
        file << eachLine << "\n";
    }
    file.close();

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
        else if(cmd == "bo1")
        {
            bool stop = false;
            broadcastStatus(argv[2], argv[3], stop);
        }
        else if(cmd == "server")
        {
            startserver( argv[2], argv[3], &datacache);
        }
        else if(cmd == "client")
        {
            auto request =  CreateTestRequest(argv[2],argv[3]);
            HandleRequestRemote(argv[4], argv[5], request);
        }
        else if (cmd == "net")
        {
            auto request = CreateTestRequest(argv[2],argv[3]);
            std::map<std::string, NodeStatus> Nodestatus;
            bool stop_recv = false;
            std::thread recvNodeStatus(recvbroadcastStatus, std::ref(Nodestatus), std::ref(stop_recv));
            double task_factor = 1.5;
            if (argc >= 6)
            {
                task_factor = boost::lexical_cast<double>(argv[5]);
            }
            auto result = HandleRequestNetwrok(request, Nodestatus, task_factor);

            std::cout << "Finished Network" << std::endl;
            stop_recv = true;

            dumpResult(result, argv[4]);
            if(recvNodeStatus.joinable())   recvNodeStatus.join();

            return 0;

        }
        else if(cmd == "tr" || cmd == "local")
        {
            auto request =  CreateTestRequest(argv[2], argv[3]);
            auto result = HandleTestRequest(request, &datacache);
              dumpResult(result, argv[4]);
            //if (argc == 5)
            //{
            //    int loop = boost::lexical_cast<int>(argv[4]);
            //    for (int i = 0; i != loop; ++i)
            //    {
            //        std::thread xxx(HandleTestRequest,request ,&datacache);
            //        xxx.join();
            //    }
            //}
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


