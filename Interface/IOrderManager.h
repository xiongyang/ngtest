#pragma once
#include "OrderData.h"

namespace BluesTrading
{
    class IOrderDataConsumer
    {
    public:
        virtual void onUpdateOrder(OrderDataDetail* orderData) = 0;
    };

    class IOrderManger
    {
    public:
        static const int32_t InvalidOrderID =  -1;
        virtual int32_t submitOrder(const OrderDataDetail& request)  = 0;		// All Request is send via request
        virtual OrderDataDetail* getOrderDetail(int32_t orderID) = 0;			// order store in the OrderManger.
        virtual void regsiterOrderDataConsumer(IOrderDataConsumer* ) = 0;
    };
}
