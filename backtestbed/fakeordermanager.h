#pragma once
#include "IOrderManager.h"
#include "testpositionmanager.h"
#include <map>
#include <vector>
#include <mutex>
#include <unordered_map>

namespace BluesTrading
{
    class FakeOrderManager :public IOrderManger
    {
    public:
        FakeOrderManager()
            : requestID_(0), posMgr_(nullptr)
        {
        }
        virtual uint64_t submitRequest(OrderRequest& request) override;		
        virtual std::vector<OrderDataDetail*> getOrderDetailByRequestID(uint64_t requestID) override;
        virtual OrderDataDetail* getOrderDetailByOrderID(uint64_t orderID) override;
        virtual std::vector<OrderDataDetail*>   getAllOrders() override;
        virtual void subscribeOrderUpdate(uint32_t updateMask, IOrderDataConsumer* consumer) override;
        virtual void unSubscribeOrderUpdate(IOrderDataConsumer* listerner) override;
    public:

        //void StartDay(uint32_t dayYYYYMMDD);
        //void EndDay(uint32_t dayYYYYMMDD);

        void MakeOrderTrade(uint64_t orderID);
        void setPosMgr(testPositionManger* mgr) {posMgr_ = mgr;}



    private:
         void handleSSECancel(OrderRequest&);
         void handleSSEModify(OrderRequest&);
         void handleSecurityNew(OrderRequest&);
         void handleFutureNew(OrderRequest& req);


         std::tuple<OrderErrorCode, ExchangeTypes> checkNewOrderRequestValid(const OrderRequest & request);
         void NotifyOrder(OrderDataDetail*);
         SenderID generateRequest();
         OrderDataDetail* generateOrder(OrderRequest&);
         std::vector<OrderDataDetail*>  generateFutureOrder(OrderRequest& req, std::tuple< OrderErrorCode,ExchangeTypes>& hint);

        
    private:
        std::map<uint64_t, OrderRequest>    requests_;
        std::map<uint64_t, OrderDataDetail*> orders_;
        uint32_t requestID_;
        uint64_t orderID_;

         testPositionManger*    posMgr_;

         std::unordered_map<IOrderDataConsumer* , uint32_t> orderdataSubscribers_;
    };



}