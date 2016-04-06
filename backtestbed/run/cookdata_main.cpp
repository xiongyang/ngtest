
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
    fixture.clean();
}

#include <array>
std::vector<DataSrcInfo> getDataSrcInfo(const std::vector< std::unordered_map<std::string, std::string> >& config)
{


    const int substr_off = strlen("DataSrc_");

    std::vector<DataSrcInfo> ret;
    DataSrcInfo currentDataSrc;

    //for (std::array<char, 256> a; steram.getline(a.data(), a.size());)
    //{
    //    std::string line(a.data());
    //    if (line.find("DataSrc_") == std::string::npos)
    //    {
    //        continue;
    //    }
    //    if (line.find("//") != 0)
    //    {
    //        continue;
    //    }

    //    auto equalindex = line.find("=");
    //    if (equalindex == std::string::npos)
    //    {
    //        continue;
    //    }
    //    const std::string key_value = line.substr(0, equalindex);
    //    const std::string value = line.substr(equalindex + 1);

    //    std::cout << "key_value  " << key_value << " " << value << std::endl;

    //    std::string subkey = key_value.substr(substr_off);
    //    if (subkey.find("Instruments") != std::string::npos)
    //    {
    //        if (currentDataSrc.instruments.size() != 0)
    //        {
    //            ret.push_back(currentDataSrc);
    //        }
    //        currentDataSrc.clear();
    //        boost::split(currentDataSrc.instruments, value, boost::algorithm::is_any_of("#"));
    //    }
    //    else if (subkey.find("StartDate") != std::string::npos)
    //    {
    //        currentDataSrc.start_date = atoi(value.c_str());
    //    }
    //    else if (subkey.find("EndDate") != std::string::npos)
    //    {
    //        currentDataSrc.end_date = atoi(value.c_str());
    //    }
    //    else if (subkey.find("Type") != std::string::npos)
    //    {
    //        currentDataSrc.datasrcType = atoi(value.c_str());
    //    }
    //    else if (subkey.find("Info") != std::string::npos)
    //    {
    //        boost::split(currentDataSrc.datasrcInfo, value, boost::algorithm::is_any_of("#"));
    //    }
    //}
    //if (currentDataSrc.instruments.size() > 0)
    //{
    //    ret.push_back(currentDataSrc);
    //}
    //return ret;

    // now only support one datasrc
    
    for (auto& each_pair : *config.begin())
    {
        const std::string& key_value = each_pair.first;
        const std::string& value = each_pair.second;
        if (key_value.find("DataSrc_") != std::string::npos)
        {
            std::string subkey = key_value.substr(substr_off);
            if (subkey.find("Instruments") != std::string::npos)
            {
                //if (currentDataSrc.instruments.size() != 0)
                //{
                //    ret.push_back(currentDataSrc);
                //    currentDataSrc.clear();
                //}
              
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


void TestThread(char ** argv)
{
    std::vector<std::string>  ourstrings;

    auto workfun = [&](int target)
    {
        double start = target;
        for (int i = 0; i != target ; ++ i)
        {
            start *= i;
            start /= i;
        }

         std::cout << "print our sum " << start << "   \n" ;
    };
    int theradNUm = atoi(argv[2]);
     std::cout << "Test Thread Num " <<  theradNUm << std::endl;
     std::vector<std::shared_ptr<std::thread> > workthread;
    for (int i = 0; i != theradNUm; ++i)
    {
        workthread.push_back(std::make_shared<std::thread>(workfun, 100000000));
    }

     std::cout << "Create All thread Don"     << std::endl;

    for (auto& each : workthread)
    {
        if(each->joinable()) each->join();
        //std::vector<std::shared_ptr<std::thread> > workthread = std::make_shared<std::thread>(workfun, 100000);
        //workthread.insert(workthread);
    }
}

boost::asio::io_service io;
void TestPostFun2(double startVal);
void TestPostFun1(double startVal)
{
    double start = startVal;
    for (int i = 0; i != 1000000 ; ++ i)
    {
        start *= i;
        start /= i;
    }
    io.post([=]{TestPostFun2(start);});
}

void TestPostFun2(double startVal)
{
    double start = startVal;
    for (int i = 0; i != 1000000 ; ++ i)
    {
        start *= i;
        start /= i;
    }
     io.post([=]{TestPostFun1(start);});
}

void TestThreadPost(char ** argv)
{

    int theradNUm = atoi(argv[2]);
    std::cout << "Test Thread Num " <<  theradNUm << std::endl;
    std::vector<std::shared_ptr<std::thread> > workthread;

    for (int i = 0; i != theradNUm; ++i)
    {
       io.post([=]{TestPostFun1(100.0);});
    }

    for (int i = 0; i != theradNUm; ++i)
    {
        workthread.push_back(std::make_shared<std::thread>([&](){
            std::cout<< "thread Create " << std::this_thread::get_id() << std::endl;
            io.run();
            std::cout << "Thread Quit "<< std::this_thread::get_id() << std::endl ;}));
    }

    std::cout << "Create All thread Don"     << std::endl;

    for (auto& each : workthread)
    {
        if(each->joinable()) each->join();
        //std::vector<std::shared_ptr<std::thread> > workthread = std::make_shared<std::thread>(workfun, 100000);
        //workthread.insert(workthread);
    }
}


TestRequest CreateTestRequest(const std::string& dllFile,  const std::string& configFile)
{
   // std::string dllFile = argv[2];
    std::string dllbytes = readFile(dllFile);

    TestRequest request;
    request.set_dllfile(dllbytes.data(), dllbytes.size());

   // = argv[3];
    std::ifstream configFileStream(configFile);
    std::vector< std::unordered_map<std::string, std::string> > allconfig = parserProps(configFileStream);
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

void session(tcp::socket sock, DataCache* datacache)
{
    try
    {
        boost::asio::streambuf buf;
        for (;;)
        {

         
            boost::asio::streambuf::mutable_buffers_type bufs = buf.prepare(1024);
            boost::system::error_code error;
            size_t n = sock.receive(bufs, 0 , error);
            buf.commit(n);

            
          //  size_t length = sock.read_some(boost::asio::buffer(data), error);
            
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            //boost::asio::write(sock, boost::asio::buffer(data, length));
        }

        std::cout << "Receive buf size "  << buf.size() << std::endl;
        std::istream remotestream(&buf);
        TestRequest request;
        bool  ret = request.ParseFromIstream(&remotestream);
        std::cout << "Receive the TestMessage "  << ret << std::endl;


      


        HandleTestRequest(request, datacache);
        std::cout << sock.remote_endpoint().address() <<":" <<  sock.remote_endpoint().port() << "  session complete \n";
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

void sendRequestToServer(const std::string& ip, const std::string& port, TestRequest& request)
{

    boost::asio::streambuf buf;
    std::ostream outstream(&buf);
    request.SerializeToOstream(&outstream);


    boost::asio::io_service io_service;
    tcp::socket sock(io_service);
    tcp::resolver resolver(io_service);
    std::cout << "try to connect to " << ip << ":" << port << std::endl;
    boost::asio::connect(sock, resolver.resolve({ip, port}));


    sock.send(buf.data());
}



// get the hardware info. and avgLoad current
void getLocalHostRuningStatus()
{
    auto ret = getCpuStatus();
    static double rttusage = ret;
    rttusage = rttusage * 0.9 + 0.1 * ret;
    std::cout << "Cpu Usage " << rttusage << "\n";
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
           sendRequestToServer(argv[4], argv[5], request);
        }
        else if(cmd == "post")
        {
            TestThreadPost(argv);
        }
        else if (cmd == "thread")
        {

            TestThread(argv);
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
		else if (cmd == "testload")
		{
			testload(argv[2]);
		}
		else if (cmd == "testload2")
		{
			std::string filename = datacache.getDataCache(5200000,20160201);
			std::cout << "DataCache Find Cache File Name " << filename << std::endl;
			testload(filename.c_str());
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


