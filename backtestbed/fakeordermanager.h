#pragma once
#include "IOrderManager.h"
#include<map>
#include<vector>

namespace BluesTrading
{
    class FakeOrderManager :public IOrderManger
    {
    public:
        FakeOrderManager()
            : senderMachineID_(0), sendStrategyID_(0),requestID_(0)
        {
        }
        virtual uint64_t submitRequest(OrderRequest& request) override;		
        virtual std::vector<OrderDataDetail*> getOrderDetailByRequestID(uint64_t requestID) override;
        virtual OrderDataDetail* getOrderDetailByOrderID(uint64_t orderID) override;
        virtual void regsiterOrderDataConsumer(IOrderDataConsumer* consumer) override;

    public:

        //void StartDay(uint32_t dayYYYYMMDD);
        //void EndDay(uint32_t dayYYYYMMDD);

        void MakeOrderTrade(uint64_t orderID);
        void SendNotify();

    private:
        std::map<uint64_t, OrderRequest>    requests_;
        std::map<uint64_t, OrderDataDetail*> orders_;

    private:
         void handleSSECancel(OrderRequest&);
         void handleSSEModify(OrderRequest&);
         void handleSSENew(OrderRequest&);

         void NotifyNewOrder(OrderDataDetail*);  //notify from another thread.

         SenderID generateRequest()
         {
             SenderID id;
             id.senderMachineID = senderMachineID_;
             id.sendStrategyID = sendStrategyID_;
             id.requestID = ++requestID_;
             return id;
         }

         OrderDataDetail* generateOrder(OrderRequest&);

         uint16_t senderMachineID_;
         uint16_t sendStrategyID_;
         uint32_t requestID_;
         IOrderDataConsumer*  consumer_;
         std::vector<OrderDataDetail>    queued_orderUpdate;

    };



}