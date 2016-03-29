#pragma once

#include "OrderData.h"

#include <string>
#include <stdio.h>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/date_time.hpp"

namespace BluesTrading
{



    struct DataSrcInfo
    {
        std::vector<std::string> instruments;
        uint32_t start_date;
        uint32_t end_date;
        uint32_t datasrcType;     // 0 mean it's the dir.  // 1 mean the MS Sql table // 2 mean mysql table 
        std::vector<std::string>    datasrcInfo;    // one or more fields for sql (eg. table name and password , user and so on) 

        void clear()
        {
            instruments.clear();
            datasrcInfo.clear();
            start_date = 0;
            end_date = 0;
            datasrcType = -1;
        }
    };

    
    boost::gregorian::date getDateFromNum(uint32_t date);
    uint32_t getNumFromDate(boost::gregorian::date date);

    uint32_t getInstrumentIndex(const std::string& instrument);

    template<typename T>
    void convert(const std::string& value, T& value_output)
    {
       value_output =  boost::lexical_cast<T>(value);
    }

    // used for stock raw data
    uint32_t getDate(const std::string& date);
    uint32_t getTimeFromDateTime(const std::string& date_time_str);
	uint32_t getTime(const std::string& date_time_str);

    std::string getTimeInMSStr(uint32_t timeInMS);
    std::string getTimeStr(uint32_t timeInMS);
    uint32_t getTimeInMS(int hour, int mins, int second, int ms = 0);

    void printOrder(std::ostream& of, const OrderDataDetail& order);

    template<typename ProcessType>
    void traverseDir(const std::string& dirName, ProcessType& process);

    template<typename MapType, typename pred>
    void remove_if_map(MapType&& c, pred&& predfun);

    std::vector< std::unordered_map<std::string, std::string> >  parserProps(std::istream& inputStr);
    std::unordered_map<std::string, std::vector<std::string>>    parserPropsSpace(std::istream& inputstream);
    std::string readFile(const std::string& file);

    double getCpuStatus();

    class UdpSenderImpl;
    class UdpSender
    {
    public:
        UdpSender(const std::string& ip, const std::string& port);

        ~UdpSender();
        uint32_t send(const std::string& buff);
    private:
        UdpSenderImpl* impl;
    };

    class UdpReceiverImpl;
    class  UdpReceiver
    {
    public:
        UdpReceiver(const std::string& ip, const std::string& port);
        ~UdpReceiver();
        uint32_t receiver(std::string* result);
    private:
        UdpReceiverImpl* impl;
    };

    class TcpSenderImpl;
    class TcpSender
    {
    public:
        TcpSender(const std::string& ip, const std::string& port);

        ~TcpSender();
        uint32_t send(const std::string& buff);
        uint32_t recv(std::string* buff);
    private:
        TcpSenderImpl* impl;
    };


    //class TcpServerImpl;
    //class    TcpServer
    //{
    //public:
    //    // listen sock
    //    TcpReceiver(const std::string& ip, const std::string& port);
    //    ~TcpReceiver();


    //private:
    //    TcpServerImpl* impl;
    //};




    class ScopeGuard
    {
    public:
        explicit ScopeGuard(std::function<void()> onExitScope)
            : onExitScope_(onExitScope), dismissed_(false)
        { }

        ~ScopeGuard()
        {
            if (!dismissed_)
            {
                onExitScope_();
            }
        }

        void Dismiss()
        {
            dismissed_ = true;
        }

    private:
        std::function<void()> onExitScope_;
        bool dismissed_;

    private: // noncopyable
        ScopeGuard(ScopeGuard const&);
        ScopeGuard& operator=(ScopeGuard const&);
    };
}

#include "util.inl"

