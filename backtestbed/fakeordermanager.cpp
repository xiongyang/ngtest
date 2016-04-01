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
        case SZE_SecurityNewOrderRequest::RequestType:
            handleSecurityNew(request);
            break;
        case    CFFEX_IndexFutureNewOrder:
        case    DCE_ProductFutureNewOrder:
        case    SHFE_ProductFutureNewOrder:
        case    CZCE_ProductFutureNewOrder:
            handleFutureNew(request);
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

    void FakeOrderManager::subscribeOrderUpdate(uint32_t updateMask, IOrderDataConsumer* consumer)
    {
        if(consumer)  orderdataSubscribers_[consumer] = updateMask;
    }

    void FakeOrderManager::unSubscribeOrderUpdate(IOrderDataConsumer* listerner)
    {
        if(listerner)  orderdataSubscribers_.erase(listerner);
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
        updateorder.orderStatus = OrderFilled;
    }

    void FakeOrderManager::handleSecurityNew(OrderRequest& requset)
    {
        assert(posMgr_ != nullptr," Not Set PosMgr");
        orders_[requset.requestID] =  generateOrder(requset);
        OrderDataDetail* targetOrder =  orders_[requset.requestID];
        NotifyOrder(targetOrder);
        if (targetOrder->sse_order.orderErrorCode != NoError)
        { 
        
            posMgr_->updateOrder(targetOrder);
            fakeTradeOrder(targetOrder);
            posMgr_->updateOrder(targetOrder);
            NotifyOrder(targetOrder);
        }
    }



    void FakeOrderManager::handleFutureNew(OrderRequest& req)
    {
          assert(posMgr_ != nullptr," Not Set PosMgr");
          std::tuple< OrderErrorCode,ExchangeTypes> ret = checkNewOrderRequestValid(req);
          auto orders =  generateFutureOrder(req);
          for (auto& orederPtr : orders)
          {
              NotifyOrder(orederPtr);
          }


          if (ret != NoError)
          {
              for (auto& targetOrder : orders)
              {
                  posMgr_->updateOrder(targetOrder);
                  fakeTradeOrder(targetOrder);
                  posMgr_->updateOrder(targetOrder);
                  NotifyOrder(targetOrder);
              }
          }
    }



    ExchangeTypes GetExchangeType(const OrderRequest &order )
    {
        ExchangeTypes ret_exch_type;
        if (order.requestType == SZE_SecurityNewOrder)
        {
            ret_exch_type = SZE;
        }
        else if(order.requestType == SSE_SecurityNewOrder)
        {
            ret_exch_type = SSE;
        }
        else if(order.requestType == CFFEX_IndexFutureNewOrder)
        {
            ret_exch_type = CFFEX;
        }
        else if(order.requestType == DCE_ProductFutureNewOrder)
        {
            ret_exch_type = DCE;
        }
        else if(order.requestType == SHFE_ProductFutureNewOrder)
        {
            ret_exch_type = SHFE;
        }
        else if(order.requestType == CZCE_ProductFutureNewOrder)
        {
            ret_exch_type = CZCE;
        }    
        return ret_exch_type;
    }

    std::tuple<OrderErrorCode, ExchangeTypes> FakeOrderManager::checkNewOrderRequestValid(const OrderRequest & order)
    {
        ExchangeTypes ret_exch_type =  GetExchangeType(order);

        if (!posMgr_)
        {
            return std::make_tuple(NoError, ret_exch_type);
        }

        switch(ret_exch_type)
        {

        case SZE:
        case SSE:
            {

                SSE_SecurityNewOrderRequest& request = order.sse_securityNewOrder;

                if(request.longshortFlag == Long)
                {
                    double need_cash = request.orderqty * request.price;
                    //  std::cout << "current cash " << posMgr_->getAccountInfo().cash << " needed cash " <<need_cash << std::endl;
                    if(posMgr_->getAccountInfo().cash  < need_cash)
                    {
                        return std::make_tuple(OrderReject_NotEnoughCash, ret_exch_type);
                    }

                }
                else if(request.longshortFlag == Short)
                {
                    CPosition::QryAmmount  qtyAmmount = posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(CPosition::LongYst);
                    if(request.orderqty > qtyAmmount.qty)
                    {
                        return std::make_tuple(OrderReject_NotEnoughInventory, ret_exch_type);
                    }
                }
                return std::make_tuple(NoError, ret_exch_type);
            }
            break;
        case CFFEX:
        case DCE:
        case SHFE:
        case CZCE:
            {
                 CFFEX_NewOrderRequest& request = order.cffex_neworderrequest;

                 //OpenCloseFlag openCloseType;
                 //OrderPriceType priceType;
                 //OrderTimeType timeType;
                 //HedgeFlag hedgeType;
                 //LongShortFlag longshortType;

                 if (request.openCloseType == Open)
                 {
                       double need_cash = request.orderqty * request.price  * 
                           InstrumentInfoFactory::getInstrumentUnitMultiplier(request.instrumentID) * 
                           InstrumentInfoFactory::getInstrumentMarginRate(request.instrumentID);

                       if(posMgr_->getAccountInfo().cash  < need_cash)
                       {
                           return std::make_tuple(OrderReject_NotEnoughCash, ret_exch_type);
                       }
                 }
                 else 
                 {
                     if(request.longshortFlag == Long)
                     {
                           CPosition::QryAmmount  qtyAmmountYst =  posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(CPosition::ShortYst);
                           CPosition::QryAmmount  qtyAmmount =  posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(CPosition::ShortToday);

                           if (request.orderqty >  qtyAmmountYst.qty + qtyAmmount.qty)
                           {
                               return std::make_tuple(OrderReject_NotEnoughInventory, ret_exch_type);
                           }
                     }
                     else if(request.longshortFlag == Short)
                     {
                         CPosition::QryAmmount  qtyAmmountYst =  posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(CPosition::LongYst);
                         CPosition::QryAmmount  qtyAmmount =  posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(CPosition::LongToday);

                         if (request.orderqty >  qtyAmmountYst.qty + qtyAmmount.qty)
                         {
                             return std::make_tuple(OrderReject_NotEnoughInventory, ret_exch_type);
                         }
                     }
                 }

 

                 return std::make_tuple(NoError, ret_exch_type);
            }
            break;
        default:
        }

    }

    void FakeOrderManager::NotifyOrder(OrderDataDetail* order)
    {
        for(auto& each_consumer : orderdataSubscribers_)
        {
           // uint32_t& ordermask = order->senderid.updateMaskID;

            //TODO how to mask 
            if (true)
            {
                each_consumer.first->onUpdateOrder(order);
            }
        }

    }

    BluesTrading::SenderID FakeOrderManager::generateRequest()
    {
        SenderID id;
        id.updateMaskID = 0;
        //id.senderMachineID = senderMachineID_;
        //id.sendStrategyID = sendStrategyID_;
        id.requestID = ++requestID_;
        return id;
    }

    void FakeOrderManager::MakeOrderTrade(uint64_t orderID)
    {
        OrderDataDetail* porder = getOrderDetailByOrderID(orderID);
        if(porder->sse_order.orderStatus !=  OrderFilled)
        {
            fakeTradeOrder(porder);
        }
    }

    BluesTrading::OrderDataDetail* FakeOrderManager::generateOrder(OrderRequest& request)
    {
        OrderDataDetail* ret = new OrderDataDetail;
  

       std::tuple< OrderErrorCode,ExchangeTypes> ret = checkNewOrderRequestValid(request);
       OrderErrorCode valid_code = ret.get<0>();
       ExchangeTypes ex_type = ret.get<1>();
        if(valid_code == NoError)
        {
            ret->requestid = request.requestID;
            ret->orderID = request.requestID;
            ret->exchangeType = ex_type;

            ret->sse_order.instrumentID = newOrderRequest.instrumentID;
            ret->sse_order.orderQty = newOrderRequest.orderqty;
            ret->sse_order.filledQty = 0;
            ret->sse_order.orderprice = newOrderRequest.price;
            ret->sse_order.tradeprice = 0;

            ret->sse_order.longshortflag = newOrderRequest.longshortflag;
            ret->sse_order.orderStatus = OrderNew;
            ret->sse_order.orderErrorCode  = valid_code;
            return ret;
        }
        else
        {
            ret->requestid = request.requestID;
            ret->orderID = request.requestID;
            ret->exchangeType = ex_type;

            ret->sse_order.instrumentID = newOrderRequest.instrumentID;
            ret->sse_order.orderQty = newOrderRequest.orderqty;
            ret->sse_order.filledQty = 0;
            ret->sse_order.orderprice = newOrderRequest.price;
            ret->sse_order.tradeprice = 0;

            ret->sse_order.longshortflag = newOrderRequest.longshortflag;
            ret->sse_order.orderStatus = OrderRejected;
            ret->sse_order.orderErrorCode  = valid_code;
            return ret;
        }

    }

    std::vector<OrderDataDetail*> FakeOrderManager::generateFutureOrder(OrderRequest& req, std::tuple< OrderErrorCode,ExchangeTypes>& hint)
    {
        std::vector<OrderDataDetail*> ret;

        CFFEX_NewOrderRequest& request = req.cffex_neworderrequest;

        if (hint.get<0>() == != NoError)
        {
            OrderDataDetail* orderDetail = new OrderDataDetail;
            orderDetail->requestid = req.requestID;
            orderDetail->orderID = req.requestID;
            orderDetail->exchangeType = hint.get<1>();

            FutureOrderDetail& order  = orderDetail->cffex_order;

            uint32_t instrumentID;
            uint32_t orderqty; 
            double price;

            OpenCloseFlag openCloseType;
            OrderPriceType priceType;
            OrderTimeType timeType;
            HedgeFlag hedgeType;
            LongShortFlag longshortType;

            OrderStatus  orderStatus;
            OrderErrorCode  orderErrorCode;

            order.instrumentID = request.instrumentID;
            order.orderqty = request.orderqty;
            order.filledQty = 0;
            order.orderprice = request.price;
            order.tradeprice = 0;

            ret->sse_order.longshortflag = newOrderRequest.longshortflag;
            ret->sse_order.orderStatus = OrderRejected;
            ret->sse_order.orderErrorCode  = valid_code;
            
        }
        return ret;
    }

}