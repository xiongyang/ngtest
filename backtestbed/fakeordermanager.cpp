#include "fakeordermanager.h"
#include <future>
#include <thread>
#include <chrono>
#include <iostream>
namespace BluesTrading
{
    struct UnknownRequestExceptionType
    {

    };
    uint64_t FakeOrderManager::submitRequest(OrderRequest& request)
    {
        request.senderID  = generateRequest();
        requests_[request.requestID] = request;

        switch(request.requestType)
        {
        case SSE_SecurityCancelRequest::RequestType:
            handleSSECancel(request);
            break;
        case SSE_SecurityModifyOrderRequest::RequestType:
            handleSSEModify(request);
            break;
        case SSE_SecurityNewOrderRequest::RequestType:
            handleSSENew(request);
            break;
        default:
            throw UnknownRequestExceptionType();
            break;
        }
        return   request.requestID;
    }

    std::vector<OrderDataDetail*> FakeOrderManager::getOrderDetailByRequestID(uint64_t requestID)
    {
        std::vector<OrderDataDetail*> ret;
        auto iter = orders_.find(requestID);
        if(iter != orders_.end())
        {
            ret.push_back(iter->second);
        }
        //ret.push_back(orders_[]);
        return ret;
    }

    OrderDataDetail* FakeOrderManager::getOrderDetailByOrderID(uint64_t orderID)
    {
        for (auto each : orders_)
        {
            if (each.second->orderID == orderID)
            {
                return each.second;
            }
        }
        return nullptr;
    }

    void FakeOrderManager::regsiterOrderDataConsumer(IOrderDataConsumer* p)
    {
           consumer_ =  p;
    }


    void FakeOrderManager::handleSSECancel( OrderRequest&)
    {
       // orders_[requset.requestID]
    }

    void FakeOrderManager::handleSSEModify(OrderRequest&)
    {

    }

    void FakeOrderManager::handleSSENew(OrderRequest& requset)
    {
        orders_[requset.requestID] =  generateOrder(requset);
        NotifyNewOrder(orders_[requset.requestID]);
        //std::async(&FakeOrderManager::NotifyNewOrder, this, orders_[requset.requestID]);
    }


    void fakeTradeOrder(OrderDataDetail* order)
    {
        SSE_OrderDetail& updateorder = order->sse_order;
        updateorder.filledQty = updateorder.orderQty;
        updateorder.tradeprice = updateorder.orderprice;
    }

    void FakeOrderManager::NotifyNewOrder(OrderDataDetail* order)
    {
      queued_orderUpdate.push_back(*order);
      //  std::this_thread::sleep_for(std::chrono::seconds(1));
      //if(consumer_)  consumer_->onUpdateOrder(order);

        //std::this_thread::sleep_for(std::chrono::seconds(1));

        //fakeTradeOrder(order);
        //if (consumer_) consumer_->onUpdateOrder(order);
    }

    void FakeOrderManager::MakeOrderTrade(uint64_t orderID)
    {
        OrderDataDetail* porder = getOrderDetailByOrderID(orderID);

        //std::this_thread::sleep_for(std::chrono::seconds(1));

        fakeTradeOrder(porder);
        queued_orderUpdate.push_back(*porder);
        //if (consumer_) consumer_->onUpdateOrder(porder);
    }


    void FakeOrderManager::SendNotify()
    {
        if (consumer_)
        {
         
            for (auto each: queued_orderUpdate)
            {
                consumer_->onUpdateOrder(&each);
           
            }
            queued_orderUpdate.clear();
        }

    }

    BluesTrading::OrderDataDetail* FakeOrderManager::generateOrder(OrderRequest& request)
    {
        OrderDataDetail* ret = new OrderDataDetail;
        const SSE_SecurityNewOrderRequest & newOrderRequest = request.sse_securityNewOrder;
        ret->requestid = request.requestID;
        ret->orderID = request.requestID;
        ret->sse_order.filledQty = 0;
        ret->sse_order.instrumentID = newOrderRequest.instrumentID;
        ret->sse_order.orderprice = newOrderRequest.price;
        ret->sse_order.isbuy = newOrderRequest.isBuy;
        ret->sse_order.orderQty = newOrderRequest.orderqty;
        ret->sse_order.tradeprice = 0;
        return ret;
    }

}