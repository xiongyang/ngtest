#include "util.h"
#include <boost/format.hpp>

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

        for (std::string line; getline(inputStr, line);)
        {
            std::vector< std::unordered_map<std::string, std::string> > before_ret = ret;
            if(line.empty() || line.size() < 3)
            {
                continue;
            }
            if(line[0] == "#" )
            {
                continue;
            }
            if (line[0] == "/" && line[1] == "/")
            {
                continue;
            }
            if(line.find('=') == line.end())
            {
                continue;
            }

            std::vector<std::string> para_value;
            boost::split(para_value, line, boost::is_any_of('='));
            if (para_value != 2)
            {
                continue;
            }

            boost::trim(para_value[0]);
            boost::trim(para_value[1]);

            std::string& prop_name = para_value[0];
            std::string& prop_value = para_value[1];

            if (prop_value[0] == '[')
            {
                std::vector<std::string> para_setp_vec;
                boost::split(para_setp_vec, prop_value, boost::is_any_of("[],: "));

                if(para_setp_vec.size() != 3)
                {
                    std::cout <<"ERROR Step Para Error " << prop_name << "  " << para_value << std::endl;
                    assert(false);
                }

                std::string& startValue = para_setp_vec[0];
                std::string& endValue = para_setp_vec[1];
                std::string& stepValue = para_setp_vec[2];

                auto isInt = [](std::string& value)
                {
                    return value.find('.')  == value.end();
                };

                if( isInt(startValue) &&  isInt(endValue) && isInt(stepValue))
                {
                    // handle int step

                    int startIntValue = atoi(startValue.c_str());
                    int endIntValue = atoi(endValue.c_str());
                    int stepValue = atoi(stepValue.c_str());

                    std::vector< std::unordered_map<std::string, std::string> > before_ret = ret;

                    for(int value = startIntValue; value <= endIntValue; value += stepValue)
                    {
                        for (auto& eachExistMap  : before_ret)
                        {
                            eachExistMap[prop_name] = itoa(value);
                        }
                    }
                }
                else
                {
                    // handle double step
                }


            }
            else if(prop_value[0] == '{')
            {

            }
            else
            {
                for (auto& eachExistMap  : ret)
                {
                    eachExistMap[prop_name] = prop_value;
                }
            }
        }
        
        return ;
    }

}