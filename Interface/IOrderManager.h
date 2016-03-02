#pragma once
#include "OrderData.h"

#include <vector>

namespace BluesTrading
{
    class IOrderDataConsumer
    {
    public:
        virtual void onUpdateOrder(OrderDataDetail* orderData) = 0;
        //virtual void getUpdateMask() = 0;
        // we may use some 
    };


    class IOrderManger
    {
    public:
        // return value is requestID equal to  request.Sender
        virtual uint64_t submitRequest(OrderRequest& request)  = 0;	

        // retrieve orders relate to the request (some request may produce many orders)
        virtual std::vector<OrderDataDetail*> getOrderDetailByRequestID(uint64_t requestID) = 0;			
        virtual OrderDataDetail* getOrderDetailByOrderID(uint64_t orderID) = 0;
        virtual std::vector<OrderDataDetail*>   getAllOrders() = 0;
        virtual void regsiterOrderDataConsumer(IOrderDataConsumer* ) = 0;
    };

    // helper help fill the request type
    template <typename OrderMangerType, typename requestDetailType>
    uint64_t submitRequest(const requestDetailType&  requestDetail, OrderMangerType*  orderManger)
    {
        OrderRequest request;
        request.requestType = requestDetailType::RequestType;
        request.fillDetail(requestDetail);
        return orderManger->submitRequest(request);
    }
    template <typename OrderMangerType>
    OrderDataDetail* getSingleOrderByOrderRequestID( OrderMangerType*  orderManger, uint64_t requestID)
    {
        std::vector<OrderDataDetail*> orders =  orderManger->getOrderDetailByRequestID(requestID);
        OrderDataDetail* firstOrder = (*orders.begin());
        return firstOrder;
    }
}
