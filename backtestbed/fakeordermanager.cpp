#include "fakeordermanager.h"
#include "util.h"

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
        case SSE_SecurityNewOrder:
        case SZE_SecurityNewOrder:
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
        updateorder.common.orderStatus = OrderFilled;
    }

    void fakeFutureTradeOrder(OrderDataDetail* order)
    {
        FutureOrderDetail& updateorder = order->cffex_order;
        updateorder.filledQty = updateorder.orderqty;
        updateorder.tradeprice = updateorder.price;
        updateorder.common.orderStatus = OrderFilled;
    }

    void FakeOrderManager::handleSecurityNew(OrderRequest& requset)
    {
        assert(posMgr_ != nullptr);
        // request.requestID == orderID;
        orders_[requset.requestID] =  generateSecurityOrder(requset);
        OrderDataDetail* targetOrder =  orders_[requset.requestID];
        NotifyOrder(targetOrder);
        if (targetOrder->sse_order.common.orderErrorCode != NoError)
        { 
            fakeTradeOrder(targetOrder);
            posMgr_->onUpdateOrder(targetOrder);
            NotifyOrder(targetOrder);
        }
    }



    void FakeOrderManager::handleFutureNew(OrderRequest& req)
    {
        assert(posMgr_ != nullptr);
        std::tuple< OrderErrorCode,ExchangeTypes> ret = checkNewOrderRequestValid(req);
        auto order_of_request =  generateFutureOrder(req, ret);
        for (auto& orderPtr : order_of_request)
        {
            orders_[orderPtr->orderID] = orderPtr;
            NotifyOrder(orderPtr);

            if (std::get<0>(ret) != NoError)
            {
                fakeFutureTradeOrder(orderPtr);
                posMgr_->onUpdateOrder(orderPtr);
                NotifyOrder(orderPtr);
            }
        }
    }



    ExchangeTypes GetExchangeType(const OrderRequest &order )
    {
        ExchangeTypes ret_exch_type;
        if (order.requestType == SZE_SecurityNewOrder)
        {
            ret_exch_type = Exch_SZE;
        }
        else if(order.requestType == SSE_SecurityNewOrder)
        {
            ret_exch_type = Exch_SSE;
        }
        else if(order.requestType == CFFEX_IndexFutureNewOrder)
        {
            ret_exch_type = Exch_CFFEX;
        }
        else if(order.requestType == DCE_ProductFutureNewOrder)
        {
            ret_exch_type = Exch_DCE;
        }
        else if(order.requestType == SHFE_ProductFutureNewOrder)
        {
            ret_exch_type = Exch_SHFE;
        }
        else if(order.requestType == CZCE_ProductFutureNewOrder)
        {
            ret_exch_type = Exch_CZCE;
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

        case Exch_SZE:
        case Exch_SSE:
            {

                const SSE_SecurityNewOrderRequest& request = order.sse_securityNewOrder;

                if(request.longshortflag == LongShortFlag_Long)
                {
                    double need_cash = request.orderqty * request.price;
                    //  std::cout << "current cash " << posMgr_->getAccountInfo().cash << " needed cash " <<need_cash << std::endl;
                    if(posMgr_->getAccountInfo().cash  < need_cash)
                    {
                        return std::make_tuple(OrderReject_NotEnoughCash, ret_exch_type);
                    }

                }
                else if(request.longshortflag == LongShortFlag_Short)
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
        case Exch_CFFEX:
        case Exch_DCE:
        case Exch_SHFE:
        case Exch_CZCE:
            {
                const CFFEX_NewOrderRequest& request = order.cffex_neworderrequest;

                //OpenCloseFlag openCloseType;
                //OrderPriceType priceType;
                //OrderTimeType timeType;
                //HedgeFlag hedgeflag;
                //LongShortFlag longshortType;

                if (request.opencloseflag == OpenCloseFlag_Open)
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
                    if(request.longshortflag == LongShortFlag_Long)
                    {
                        CPosition::QryAmmount  qtyAmmountYst =  posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(CPosition::ShortYst);
                        CPosition::QryAmmount  qtyAmmount =  posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(CPosition::ShortToday);

                        if (request.orderqty >  qtyAmmountYst.qty + qtyAmmount.qty)
                        {
                            return std::make_tuple(OrderReject_NotEnoughInventory, ret_exch_type);
                        }
                    }
                    else if(request.longshortflag ==  LongShortFlag_Short)
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
            break;
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
        id.requestID =requestID_;
        requestID_ += 2;  // because sometime on request gennrate 2 order
        return id;
    }

    void FakeOrderManager::MakeOrderTrade(uint64_t orderID)
    {
        OrderDataDetail* porder = getOrderDetailByOrderID(orderID);
        if(porder->sse_order.common.orderStatus !=  OrderFilled)
        {
            fakeTradeOrder(porder);
        }
    }

    BluesTrading::OrderDataDetail* FakeOrderManager::generateSecurityOrder(OrderRequest& request)
    {
        OrderDataDetail* ret = new OrderDataDetail;


        std::tuple< OrderErrorCode,ExchangeTypes> ret_check = checkNewOrderRequestValid(request);
        OrderErrorCode valid_code = std::get<0>(ret_check);
        ExchangeTypes ex_type = std::get<1>(ret_check);
        SSE_SecurityNewOrderRequest& order = request.sse_securityNewOrder;

        if(valid_code == NoError)
        {
            ret->requestid = request.requestID;
            ret->orderID = request.requestID;
            ret->exchangeType = ex_type;

            ret->sse_order.instrumentID = order.instrumentID;
            ret->sse_order.orderQty = order.orderqty;
            ret->sse_order.filledQty = 0;
            ret->sse_order.orderprice = order.price;
            ret->sse_order.tradeprice = 0;

            ret->sse_order.longshortflag = order.longshortflag;
            ret->sse_order.common.orderStatus = OrderNew;
            ret->sse_order.common.orderErrorCode  = valid_code;
            return ret;
        }
        else
        {
            ret->requestid = request.requestID;
            ret->orderID = request.requestID;
            ret->exchangeType = ex_type;

            ret->sse_order.instrumentID = order.instrumentID;
            ret->sse_order.orderQty = order.orderqty;
            ret->sse_order.filledQty = 0;
            ret->sse_order.orderprice = order.price;
            ret->sse_order.tradeprice = 0;

            ret->sse_order.longshortflag = order.longshortflag;
            ret->sse_order.common.orderStatus = OrderRejected;
            ret->sse_order.common.orderErrorCode  = valid_code;
            return ret;
        }

    }

    std::vector<OrderDataDetail*> FakeOrderManager::generateFutureOrder(OrderRequest& req, std::tuple< OrderErrorCode,ExchangeTypes>& hint)
    {
        std::vector<OrderDataDetail*> ret;

        CFFEX_NewOrderRequest& request = req.cffex_neworderrequest;

        if (std::get<0>(hint) != NoError)
        {
            OrderDataDetail* orderDetail = new OrderDataDetail;
            orderDetail->requestid = req.requestID;
            orderDetail->orderID = req.requestID;
            orderDetail->exchangeType = std::get<1>(hint);

            FutureOrderDetail& order  = orderDetail->cffex_order;
            order.instrumentID = request.instrumentID;
            order.orderqty = request.orderqty;
            order.price = request.price;
            order.tradeprice  = 0;
            order.filledQty = 0;
            order.opencloseflag = request.opencloseflag;
            order.pricetype = request.pricetype;
            order.timetype = request.timetype;
            order.hedgeflag = request.hedgeflag;
            order.longshortflag = request.longshortflag;
            order.common.orderErrorCode = std::get<0>(hint);
            order.common.orderStatus = OrderRejected;

            ret.push_back(orderDetail);

        }
        else
        {
            if (request.opencloseflag != OpenCloseFlag_Open)
            {
                CPosition::PositionType ystPosType;
                CPosition::PositionType todayPosType;
                if (request.longshortflag == LongShortFlag_Long)
                {
                    ystPosType = CPosition::ShortYst;
                    todayPosType = CPosition::ShortToday;
                }
                else
                {
                    ystPosType = CPosition::LongYst;
                    todayPosType = CPosition::LongToday;
                }
                CPosition::QryAmmount  qtyAmmountYst =  posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(ystPosType);
                CPosition::QryAmmount  qtyAmmount =  posMgr_->getPosition(request.instrumentID).getTotalQtyAmmount(todayPosType);
                uint32_t yst_qty = qtyAmmountYst.qty;
                uint32_t today_qty = qtyAmmount.qty;

                if (today_qty > 0)
                {
                    uint32_t close_today_qty = std::min(today_qty, request.orderqty);

                    OrderDataDetail* orderDetail = new OrderDataDetail;
                    orderDetail->requestid = req.requestID;
                    orderDetail->orderID = req.requestID;
                    orderDetail->exchangeType = std::get<1>(hint);

                    FutureOrderDetail& order  = orderDetail->cffex_order;
                    order.instrumentID = request.instrumentID;
                    order.orderqty = close_today_qty;
                    order.price = request.price;
                    order.tradeprice  = 0;
                    order.filledQty = 0;
                    order.opencloseflag  = OpenCloseFlag_CloseToday;
                    order.pricetype = request.pricetype;
                    order.timetype = request.timetype;
                    order.hedgeflag = request.hedgeflag;
                    order.longshortflag = request.longshortflag;
                    order.common.orderErrorCode = std::get<0>(hint);
                    order.common.orderStatus = OrderNew;
                    ret.push_back(orderDetail);


                    request.orderqty -= close_today_qty;
                }

                if (request.orderqty > 0)
                {

                    OrderDataDetail* orderDetail = new OrderDataDetail;
                    orderDetail->requestid = req.requestID;
                    orderDetail->orderID = req.requestID + 1;
                    orderDetail->exchangeType =std::get<1>(hint);

                    FutureOrderDetail& order  = orderDetail->cffex_order;
                    order.instrumentID = request.instrumentID;
                    order.orderqty = request.orderqty;
                    order.price = request.price;
                    order.tradeprice  = 0;
                    order.filledQty = 0;
                    order.opencloseflag = OpenCloseFlag_CloseYst;
                    order.pricetype = request.pricetype;
                    order.timetype = request.timetype;
                    order.hedgeflag = request.hedgeflag;
                    order.longshortflag = request.longshortflag;
                    order.common.orderErrorCode =std::get<0>(hint);
                    order.common.orderStatus = OrderNew;
                    ret.push_back(orderDetail);
                }


            }
            else
            {
                // open

                OrderDataDetail* orderDetail = new OrderDataDetail;
                orderDetail->requestid = req.requestID;
                orderDetail->orderID = req.requestID;
                orderDetail->exchangeType =  std::get<1>(hint);

                FutureOrderDetail& order  = orderDetail->cffex_order;
                order.instrumentID = request.instrumentID;
                order.orderqty = request.orderqty;
                order.price = request.price;
                order.tradeprice  = 0;
                order.filledQty = 0;
                order.opencloseflag = request.opencloseflag;
                order.pricetype = request.pricetype;
                order.timetype = request.timetype;
                order.hedgeflag = request.hedgeflag;
                order.longshortflag = request.longshortflag;
                order.common.orderErrorCode = std::get<0>(hint);
                order.common.orderStatus = OrderNew;
                ret.push_back(orderDetail);
            }


        }
        return ret;
    }

}