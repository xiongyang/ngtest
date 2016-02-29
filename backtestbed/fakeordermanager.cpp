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

    std::vector<OrderDataDetail*> FakeOrderManager::getAllOrders()
    {
         std::vector<OrderDataDetail*> ret;
         for (auto each : orders_)
         {
               ret.push_back(each.second);
         }
         return ret;
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


    void fakeTradeOrder(OrderDataDetail* order)
    {
        SSE_OrderDetail& updateorder = order->sse_order;
        updateorder.filledQty = updateorder.orderQty;
        updateorder.tradeprice = updateorder.orderprice;
        updateorder.orderStatus = SSE_OrderDetail::SSE_OrderTraded;
    }

    void FakeOrderManager::handleSSENew(OrderRequest& requset)
    {
        std::cout << "handleSSE New \n" ;
        orders_[requset.requestID] =  generateOrder(requset);
        //NotifyNewOrder(orders_[requset.requestID]);
        //we can not update order on another thread. because the process tick is too fast

        consumer_->onUpdateOrder(orders_[requset.requestID]);
        fakeTradeOrder(orders_[requset.requestID]);
        consumer_->onUpdateOrder(orders_[requset.requestID]);
    }



    void FakeOrderManager::NotifyNewOrder(OrderDataDetail* order)
    {
        std::lock_guard<std::mutex> guard(updateMutex_);
        queued_orderUpdate.push_back(*order);
    }

    void FakeOrderManager::MakeOrderTrade(uint64_t orderID)
    {
        OrderDataDetail* porder = getOrderDetailByOrderID(orderID);
        if(porder->sse_order.orderStatus != SSE_OrderDetail::SSE_OrderTraded)
        {
            fakeTradeOrder(porder);
            //{
            //    std::lock_guard<std::mutex> guard(updateMutex_);
            //    queued_orderUpdate.push_back(*porder);
            //}
        }
    }


    void FakeOrderManager::SendNotify()
    {
        if (consumer_)
        {
           std::vector<OrderDataDetail> nofiyorders;

           {
                  std::lock_guard<std::mutex> guard(updateMutex_);
                  nofiyorders.swap(queued_orderUpdate);
           }
        

            for (auto each: nofiyorders)
            {
                consumer_->onUpdateOrder(&each);
            }
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
        ret->exchangeType = SSE;
        ret->sse_order.orderStatus = SSE_OrderDetail::SSE_OrderNew;
        return ret;
    }

}