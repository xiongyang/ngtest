#include "util.h"
#include <boost/format.hpp>

namespace BluesTrading
{

     void printSSEOrder(std::ostream& of, const OrderDataDetail& order)
     {
         of << boost::format("[SSEOrder OID:%1% Inst:%2% Price:%3% Qty:%4% Traded:%5% LastTradePrice:%6% Status:%7$d]\n") 
             % order.orderID   % order.sse_order.instrumentID % order.sse_order.orderprice % order.sse_order.orderQty % order.sse_order.filledQty
             % order.sse_order.tradeprice % int(order.sse_order.orderStatus);
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

}