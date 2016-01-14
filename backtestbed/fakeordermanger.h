#pragma once

#include "IOrderManager.h"

namespace BluesTrading
{
    class FakeOrderManager : public IOrderManger
    {
    public:
        virtual int32_t submitOrder(const OrderDataDetail& request) override;		// All Request is send via request
        virtual OrderDataDetail* getOrderDetail(int32_t orderID) override;			// order store in the OrderManger.
        virtual void regsiterOrderDataConsumer(IOrderDataConsumer* ) override;
    };
}