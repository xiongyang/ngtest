#pragma once
#include <string>
#include <stdio.h>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "OrderData.h"
#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"

namespace BluesTrading
{

    template<typename T>
    void convert(const std::string& value, T& value_output)
    {
       value_output =  boost::lexical_cast<T>(value);
    }

    // used for stock raw data
    uint32_t getDate(const std::string& date);
    uint32_t getTime(const std::string& date_time_str);

    std::string getTimeInMSStr(uint32_t timeInMS);

    const std::string getTimeStr(uint32_t timeInMS);

    void printOrder(std::ostream& of, const OrderDataDetail& order);

    template<typename ProcessType>
    void traverseDir(const std::string& dirName, ProcessType& process);

    template<typename MapType, typename pred>
    void remove_if_map(MapType&& c, pred&& predfun);

    std::vector< std::unordered_map<std::string, std::string> > parserProps(std::istream& inputStr);
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

