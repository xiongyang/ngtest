#include "util.h"

#include <fstream>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#ifdef _BL_WIN32_PLATFROM_
#include <windows.h>
#include <Pdh.h>
//#pragma comment(lib,"pdh.dll")
#include "dynamicloader.h"
#include <thread>
#include <mutex>
#endif



namespace BluesTrading
{

    void printSSEOrder(std::ostream& of, const OrderDataDetail& order)
    {
        of << boost::format("[SSEOrder OID:[%10% %1% Inst:%2% Price:%3% Qty:%4% Dir:%9% Traded:%5% LastTradePrice:%6% Status:%7$d ErrorCode:%8%]\n") 
            % order.orderID   % order.sse_order.instrumentID % order.sse_order.orderprice % order.sse_order.orderQty % order.sse_order.filledQty
            % order.sse_order.tradeprice % int(order.sse_order.orderStatus) % int(order.sse_order.orderErrorCode) % order.sse_order.isbuy
            % order.senderid.requestID ;
    }

    std::string getTimeInMSStr(uint32_t timeInMS)
    {
        char pTimeBuf[64];
        int milliSecond = timeInMS % 1000;
        int second = (timeInMS - milliSecond) / 1000; second = second % 60;
        int minute = ((timeInMS - milliSecond) / 1000 - second) / 60; minute = minute % 60;
        int hour = (((timeInMS - milliSecond) / 1000 - second) / 60 - minute) / 60;

        sprintf(pTimeBuf, "%02d:%02d:%02d.%03d", hour, minute, second, milliSecond);
        return pTimeBuf;
    }

    void printOrder(std::ostream& of, const OrderDataDetail& order)
    {
        switch(order.exchangeType)
        {

        case  SSE:
            printSSEOrder(of, order);
            break;
        case     SZE:
        case     SHFE:
        case     CZCE:
        case    CFFEX:
        case    DCE:
            of << "Not Implement Yet Print OrderYet \n" ;
            break;
        default:
            of << "Unknown OrderType \n" ;
            break;
        }
    }

    const std::string getTimeStr(uint32_t timeInMS)
    {
        uint32_t ms = timeInMS % 1000;
        uint32_t s = timeInMS  / 1000 % 60;
        uint32_t min = timeInMS / (60 * 1000) % 60;
        uint32_t hour = timeInMS / (60 * 60 * 1000);
        return (boost::format("%02d:%02d:%02d.%02d") % hour % min % s % ms).str();
    }

    std::vector< std::unordered_map<std::string, std::string> > parserProps(std::istream& inputStr)
    {
        std::vector< std::unordered_map<std::string, std::string> > ret;
        std::unordered_map<std::string, std::vector<std::string>> space = parserPropsSpace(inputStr);
        uint32_t totalSpace = 1;
        for (auto& each_pair : space)
        {
            totalSpace *= each_pair.second.size();
        }

        ret.resize(totalSpace);

        for (auto& each_pair : space)
        {
            const std::string& prop_name = each_pair.first;
            const std::vector<std::string>& prop_vals = each_pair.second;
            int prop_size = prop_vals.size();

            for (int i = 0 ; i != ret.size(); ++i)
            {
                ret[i][prop_name] =  prop_vals[i % prop_size];
            }
        }

        return ret;
    }

    template<typename ValueType>
    std::vector<std::string> parse_set (const std::string& startValueStr, const std::string& endValueStr, const std::string& stepValueStr)
    {
        std::vector<std::string> ret;
        ValueType startIntValue = boost::lexical_cast<ValueType>(startValueStr);
        ValueType endIntValue =  boost::lexical_cast<ValueType>(endValueStr);
        ValueType stepValue =  boost::lexical_cast<ValueType>(stepValueStr);
        for(ValueType value = startIntValue; value <= endIntValue + 0.000001 ; value += stepValue)
        {
            ret.emplace_back( boost::lexical_cast<std::string>(value));
        }
        return ret;
    };

    std::unordered_map<std::string, std::vector<std::string>> parserPropsSpace(std::istream& inputStr)
    {

        //std::vector< std::unordered_map<std::string, std::string> > ret;

        std::unordered_map<std::string, std::vector<std::string>> paras_for_every_prop;

        for (std::string line; getline(inputStr, line);)
        {
            std::vector<std::string> prop_values;
            if(line.empty() || line.size() < 3)
            {
                continue;
            }
            if(line[0] == '#' )
            {
                continue;
            }
            if (line[0] == '/' && line[1] == '/')
            {
                continue;
            }
            if(line.find('=') == std::string::npos)
            {
                continue;
            }

            std::vector<std::string> para_value_pair;
            boost::split(para_value_pair, line, boost::is_any_of("="));
            if (para_value_pair.size() != 2)
            {
                continue;
            }

            boost::trim(para_value_pair[0]);
            boost::trim(para_value_pair[1]);

            std::string& prop_name = para_value_pair[0];
            std::string& prop_value = para_value_pair[1];

            if (prop_value[0] == '[')
            {
                std::vector<std::string> para_setp_vec;
                boost::trim_if(prop_value, boost::is_any_of("[]{}"));
                boost::split(para_setp_vec, prop_value, boost::algorithm::is_any_of(",: "), boost::token_compress_on);

                if(para_setp_vec.size() != 3)
                {
                    std::cout <<"ERROR Step Para Error " << prop_name << "  " << prop_value << " " << para_setp_vec.size() <<std::endl;
                    assert(false);
                }

                std::cout << std::endl;

                std::string& startValue = para_setp_vec[0];
                std::string& endValue = para_setp_vec[1];
                std::string& stepValue = para_setp_vec[2];

                auto isInt = [](std::string& value)
                {
                    return value.find('.')  == std::string::npos;
                };



                if( isInt(startValue) &&  isInt(endValue) && isInt(stepValue))
                {
                    // handle int step
                    prop_values  = parse_set<int>(startValue, endValue, stepValue);
                }
                else
                {
                    prop_values  = parse_set<double>(startValue, endValue, stepValue);
                }

                paras_for_every_prop[prop_name] = prop_values;


            }
            else if(prop_value[0] == '{')
            {
                boost::trim_if(prop_value, boost::is_any_of("[]{}"));
                std::vector<std::string> para_set_vec;
                boost::split(para_set_vec, prop_value, boost::algorithm::is_any_of(",: "));
                paras_for_every_prop[prop_name] = std::move(para_set_vec);
            }
            else
            {
                paras_for_every_prop[prop_name] = std::vector<std::string> { prop_value};
            }
        }

        return paras_for_every_prop;
    }


    std::string readFile(const std::string& file)
    {
        std::fstream filestr(file , std::ios_base::binary | std::ios_base::in);

        filestr.seekg(0, std::ios_base::end);
        size_t filesize = filestr.tellg();

        std::string ret;
        ret.resize(filesize);

        filestr.seekg(0);
        filestr.read((char*)ret.data(), filesize);

        return ret;
    }





#ifdef _BL_WIN32_PLATFROM_

    PDH_HQUERY query;
    PDH_STATUS status;
    PDH_HCOUNTER counter;
    HANDLE Event;
    std::once_flag flag1;

    void InitCpuUsage()
    {

        auto PdhOpenQueryfun =  GetSharedLibFun<decltype(PdhOpenQuery)>("Pdh.dll", "PdhOpenQuery");

        auto PdhAddCounterfun = GetSharedLibFun< decltype(PdhAddCounterA) >("Pdh.dll", "PdhAddCounterA");

        PdhOpenQueryfun(NULL, 0, &query);
        if (status != ERROR_SUCCESS)
        {
            std::cout << "OpenQuery Fail " << std::endl;

        }

        Event = CreateEvent(NULL, FALSE, FALSE, "MyEvent");

        //std::cout << "1" << std::endl;

        status = PdhAddCounterfun(query, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &counter);
        if (status != ERROR_SUCCESS)
        {
            std::cout << "PdhAddCounterfun Fail " << std::endl;
        }
    }


   double getCpuStatus()
    {

        std::call_once(flag1, InitCpuUsage);

        auto PdhCollectQueryDatafun =   GetSharedLibFun<decltype(PdhCollectQueryDataEx)>("Pdh.dll", "PdhCollectQueryDataEx");
        auto PdhGetFormattedCounterValuefun = GetSharedLibFun<decltype(PdhGetFormattedCounterValue)>("Pdh.dll", "PdhGetFormattedCounterValue");



        status =  PdhCollectQueryDatafun(query, 1, Event);

        if(status != ERROR_SUCCESS)
        {
            std::cout << "PdhCollectQueryDatafun Fail " << std::endl;
            return 0;
        }

        auto WaitResult = WaitForSingleObject(Event, INFINITE);
        ULONG CounterType;
        PDH_FMT_COUNTERVALUE DisplayValue;

        if (WaitResult == WAIT_OBJECT_0)
        {
            PdhGetFormattedCounterValuefun(counter,PDH_FMT_DOUBLE, &CounterType, &DisplayValue);
        }
        return  DisplayValue.doubleValue;
    }
#endif
#ifdef _BL_LINUX_PLATFROM_

    void InitCpuUsage()
    {

    }

    double getCpuStatus()
    {
        std::cout << "xxxx" << std::endl;
        return  0.0;
    }
#endif

    class UdpSenderImpl
    {
    public:
        UdpSenderImpl(const std::string& ip , const std::string& port)
            :socket_(io, boost::asio::ip::udp::v4())
        {
            boost::asio::ip::udp::resolver resolver(io);
            endpoint_ = *resolver.resolve({ boost::asio::ip::udp::v4(), ip, port });
        }

        uint32_t send(const std::string& buff)
        {
            uint32_t send_size = socket_.send_to(boost::asio::buffer(buff), endpoint_);
            return send_size;
        }

    private:
        boost::asio::io_service io;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint endpoint_;
    };

    UdpSender::UdpSender(const std::string & ip, const std::string&  port)
    {
        try
        {
            impl = new UdpSenderImpl(ip, port);
        }
        catch (std::exception& e)
        {
            std::cout << "xxx " << e.what() << std::endl;
        }
    }


    UdpSender::~UdpSender()
    {
        delete impl;
    }

    uint32_t UdpSender::send(const std::string& buff)
    {
       return  impl->send(buff);
    }

    UdpReceiver::UdpReceiver(const std::string& ip, const std::string& port)
    {

    }

    UdpReceiver::~UdpReceiver()
    {

    }

}