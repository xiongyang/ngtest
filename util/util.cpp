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
            % order.sse_order.tradeprice % int(order.sse_order.common.orderStatus) % int(order.sse_order.common.orderErrorCode) % order.sse_order.longshortflag
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

    std::string getTimeStr(uint32_t timeInMS)
    {
        uint32_t ms = timeInMS % 1000;
        uint32_t s = timeInMS  / 1000 % 60;
        uint32_t min = timeInMS / (60 * 1000) % 60;
        uint32_t hour = timeInMS / (60 * 60 * 1000);
        return (boost::format("%02d:%02d:%02d.%03d") % hour % min % s % ms).str();
    }



    std::uint32_t getTimeInMS(int hour, int mins, int second, int ms /*= 0*/)
    {
        return hour * 3600 * 1000 + mins * 60 * 1000 + second * 1000 + ms ;
    }

    void printOrder(std::ostream& of, const OrderDataDetail& order)
    {
        switch(order.exchangeType)
        {

        case  Exch_SSE:
            printSSEOrder(of, order);
            break;
        case     Exch_SZE:
        case     Exch_SHFE:
        case     Exch_CZCE:
        case    Exch_CFFEX:
        case    Exch_DCE:
			//of << order.cff
           // of << "Not Implement Yet Print OrderYet \n" ;
            break;
        default:
            of << "Unknown OrderType \n" ;
            break;
        }
    }


    std::vector< std::map<std::string, std::string> > parserProps(std::istream& inputStr)
    {
        std::vector< std::map<std::string, std::string> > ret;
        std::map<std::string, std::vector<std::string>> space = parserPropsSpace(inputStr);
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

    std::map<std::string, std::vector<std::string>> parserPropsSpace(std::istream& inputStr)
    {

        //std::vector< std::unordered_map<std::string, std::string> > ret;

        std::map<std::string, std::vector<std::string>> paras_for_every_prop;

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

    CDynamicLibrary* g_pdh_library;
    void InitCpuUsage()
    {
        g_pdh_library = new CDynamicLibrary("Pdh.dll");  
       //let it leak. because it only once
        auto PdhOpenQueryfun =  GetSharedLibFun<decltype(PdhOpenQuery)>(g_pdh_library, "PdhOpenQuery");

        auto PdhAddCounterfun = GetSharedLibFun< decltype(PdhAddCounterA) >(g_pdh_library, "PdhAddCounterA");

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

        return;
    }


   double getCpuStatus()
    {

        std::call_once(flag1, InitCpuUsage);

        auto PdhCollectQueryDatafun =   GetSharedLibFun<decltype(PdhCollectQueryDataEx)>(g_pdh_library, "PdhCollectQueryDataEx");
        auto PdhGetFormattedCounterValuefun = GetSharedLibFun<decltype(PdhGetFormattedCounterValue)>(g_pdh_library, "PdhGetFormattedCounterValue");



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

    class UdpReceiverImpl
    {
    public:
        UdpReceiverImpl(const std::string& ip , const std::string& port)
            :socket_(io, boost::asio::ip::udp::v4())
        {
            boost::asio::ip::udp::resolver resolver(io);
            endpoint_ = *resolver.resolve({ boost::asio::ip::udp::v4(), ip, port });
        }

        uint32_t recv(std::string& buff)
        {
            buff.resize(2000);
            uint32_t recv_size = socket_.receive_from(boost::asio::buffer(&buff.front(), buff.size()), endpoint_);
            buff.resize(recv_size);
            return recv_size;
        }
    private:
        boost::asio::io_service io;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint endpoint_;
    };


    UdpReceiver::UdpReceiver(const std::string& ip, const std::string& port)
    {
        impl = new UdpReceiverImpl(ip, port);
    }

    UdpReceiver::~UdpReceiver()
    {
        delete impl;
    }

    uint32_t UdpReceiver::receiver(std::string* result)
    {
        if (result == nullptr)
        {
            return 0;
        }
        else
        {
            return impl->recv(*result);
        }
    }


    class TcpSenderImpl
    {
    public:
        TcpSenderImpl(const std::string& ip , const std::string& port)
            :socket_(io)
        {
            boost::asio::ip::udp::resolver resolver(io);
            endpoint_ = *resolver.resolve({ ip, port });
            socket_.connect(endpoint_);
        }

        uint32_t send(const std::string& buff)
        {
//           return  boost::asio::write(socket_, boost::asio::buffer(buff));

            uint32_t send_size = socket_.send(boost::asio::buffer(buff));
            return send_size;
        }

        uint32_t recv(std::string& buff)
        {
            buff.resize(2000);

            uint32_t recv_size =  socket_.receive(boost::asio::buffer(&buff.front(), buff.size()));
            buff.resize(recv_size);
            return recv_size;
        }

    private:
        boost::asio::io_service io;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint endpoint_;
    };

    TcpSender::TcpSender(const std::string & ip, const std::string&  port)
    {
        try
        {
            impl = new TcpSenderImpl(ip, port);
        }
        catch (std::exception& e)
        {
            std::cout << "xxx " << e.what() << std::endl;
        }
    }


    TcpSender::~TcpSender()
    {
        delete impl;
    }

    uint32_t TcpSender::send(const std::string& buff)
    {
        return  impl->send(buff);
    }

    std::uint32_t TcpSender::recv(std::string* result)
    {
        if (result == nullptr)
        {
            return 0;
        }
        else
        {
            return impl->recv(*result);
        }
    }




    uint32_t getDate(const std::string& date)
    {
        std::vector<std::string> strings;
        boost::split(strings, date, boost::is_any_of("- "));
        assert(strings.size() >= 3);

        return atoi(strings[0].c_str())  * 10000  + atoi(strings[1].c_str()) * 100 + atoi(strings[2].c_str()) ;
    }

    uint32_t getTimeFromDateTime(const std::string& dateTime)
    {
        //2013-10-10 09:15:05
        std::vector<std::string> fields;
        boost::split(fields, dateTime, boost::is_any_of(" :"));
        return atoi(fields[1].c_str()) * 3600 * 1000 + atoi(fields[2].c_str()) * 60 * 1000 +  atoi(fields[3].c_str()) *  1000;
        //return 0;
    } 

	 uint32_t getTime(const std::string& date_time_str)
	 {
         std::vector<std::string> fields;
         boost::split(fields, date_time_str, boost::is_any_of(" :"));
         return atoi(fields[0].c_str()) * 3600 * 1000 + atoi(fields[1].c_str()) * 60 * 1000 +  atoi(fields[2].c_str()) *  1000;
	 }




     uint32_t getInstrumentIndex(const std::string& instrument)
    {
        if (instrument == "ag")
        {
            return 5200000;
        }
        else if (instrument == "rb")
        {
            return 5210000;
        }
        else if (instrument == "au")
        {
            return 5220000;
        }
        else if (instrument == "bu")
        {
            return 5230000;
        }
        else if (instrument == "cu")
        {
            return 5240000;
        }
        else if (instrument == "hc")
        {
            return 5250000;
        }
        else if (instrument == "ni")
        {
            return 5260000;
        }
        else if (instrument == "ru")
        {
            return 5270000;
        }
        else if (instrument == "zn")
        {
            return 5280000;
        }
        else if (instrument == "a")
        {
            return 5290000;
        }
        else if (instrument == "c")
        {
            return 5300000;
        }
        else if (instrument == "cs")
        {
            return 5310000;
        }
        else if (instrument == "i")
        {
            return 5320000;
        }
        else if (instrument == "j")
        {
            return 5330000;
        }
        else if (instrument == "jm")
        {
            return 5340000;
        }
        else if (instrument == "jd")
        {
            return 5350000;
        }
        else if (instrument == "l")
        {
            return 5360000;
        }
        else if (instrument == "m")
        {
            return 5370000;
        }
        else if (instrument == "p")
        {
            return 5380000;
        }
        else if (instrument == "pp")
        {
            return 5390000;
        }
        else if (instrument == "y")
        {
            return 5400000;
        }
        else if (instrument == "CF")
        {
            return 5410000;
        }
        else if (instrument == "TA")
        {
            return 5420000;
        }
        else if (instrument == "TA")
        {
            return 5430000;
        }
        else if (instrument == "SR")
        {
            return 5440000;
        }
        else if (instrument == "FG")
        {
            return 5450000;
        }
        else if (instrument == "RM")
        {
            return 5460000;
        }
        else if (instrument == "MA")
        {
            return 5470000;
        }
        else
        {
            return 0;
        }
    }

    std::tuple<uint32_t, uint32_t,uint32_t> spliteYearMonthDay(uint32_t date)
    {
        uint32_t start_year = date / 10000;
        uint32_t start_month = ( date - start_year * 10000) / 100;
        uint32_t start_day = date % 100;
        return std::make_tuple(start_year, start_month, start_day);
    }

    boost::gregorian::date getDateFromNum(uint32_t date)
    {
       auto datetuple = spliteYearMonthDay(date);
       return boost::gregorian::date( std::get<0>(datetuple), std::get<1>(datetuple), std::get<2>(datetuple));
    }


    uint32_t getNumFromDate(boost::gregorian::date date)
    {
        return boost::lexical_cast<uint32_t>( boost::gregorian::to_iso_string(date));
    }

    uint32_t InstrumentInfoFactory::getInstrumentUnitMultiplier(uint32_t inst)
    {
		if(inst == 5270000)
			return 10;
		else if(inst == 5200000)
			return 15;
		else if(inst == 5340000)
			return 60;
		else if(inst == 5350000)
			return 10;
    }

    double InstrumentInfoFactory::getInstrumentMarginRate(uint32_t inst)
    {
        if(inst == 5270000)
			return 0.1;
		else if(inst == 5200000)
			return 0.1;
		else if(inst == 5340000)
			return 0.1;
		else if(inst == 5250000)
			return 0.1;
    }

	double	InstrumentInfoFactory::getInstrumentTickSize(uint32_t inst)
	{
		if(inst == 5270000)
			return 5;
		else if(inst == 5200000)
			return 1;
		else if(inst == 5340000)
			return 0.5;
		else if(inst == 5250000)
			return 1;
	}
	double	InstrumentInfoFactory::getInstrumentCommision(uint32_t inst)
	{
		if(inst == 5270000)
			return 30;
		else if(inst == 5200000)
			return 5;
		else if(inst == 5340000)
			return 9;
		else if(inst == 5250000)
			return 5.4;
	}
	double	InstrumentInfoFactory::getInstrumentLimit(uint32_t inst)
	{
		if(inst == 5270000)
			return 0.05;
		else if(inst == 5200000)
			return 1;
		else if(inst == 5340000)
			return 0.04;
		else if(inst == 5250000)
			return 0.05;
	}


}