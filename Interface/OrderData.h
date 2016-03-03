#pragma once

#include <cstdint>
#include <string.h>

namespace BluesTrading
{
    enum RequestTypeEnum
    {
        SSESZE_LevelFundCancel,
        SSESZE_LevelFundBuy,
        SSESZE_LevelFundSell,
        SSESZE_LevelFundMerge,
        SSESZE_LevelFundSplit,

        SSE_SecurityCancel ,
        SSE_SecurityNewOrder,
        SSE_SecurityModify,

        SSE_OptionsCancel,
        SSE_OptionsNewOrder,
        SSE_OptionsModify,

        CFFEX_IndexFutureCancel,
        CFFEX_IndexFutureNewOrder,
        CFFEX_IndexFutureModify,
      
        DCE_ProductFutureCancel,
        DCE_ProductFutureNewOrder,
        DCE_ProductFutureModify,

        SHFE_ProductFutureCancel,
        SHFE_ProductFutureNewOrder,
        SHFE_ProductFutureModify,

        CZCE_ProductFutureCancel,
        CZCE_ProductFutureNewOrder,
        CZCE_ProductFutureModify,

        // more exchange more instrument types
    };

    //enum OrderStatus
    //{
    //    PendingNew,
    //    New,
    //    Parti
    //};

    enum ExchangeTypes
    {
        CFFEX,
        DCE,
        SSE,
        SZE,
        SHFE,
        CZCE
    };

    struct SenderID
    {
        uint16_t senderMachineID;
        uint16_t sendStrategyID;
        uint32_t requestID;
    };

    struct SSE_SecurityCancelRequest
    {
        static const int RequestType = SSE_SecurityCancel;
        uint64_t blueOrderID;
    };

    struct  SSE_SecurityModifyOrderRequest
    {
        static const int RequestType = SSE_SecurityModify;
        uint32_t instrumentID;
        double price;
        uint8_t orderType;
        uint8_t priceType;
        uint8_t diretion;
     
        uint32_t orderqty; 
    };

    struct SSE_SecurityNewOrderRequest
    {
        static const int RequestType = SSE_SecurityNewOrder;
        uint32_t instrumentID;
        double price;
        uint32_t orderqty; 
        uint8_t orderType;
        uint8_t priceType;
        bool isBuy;
    };

    struct SSE_OrderDetail
    {
        enum 
        {
            SSE_OrderPendingNew = 0,
            SSE_OrderNew,
            SSE_OrderRejected,
            SSE_OrderCanceled,
            SSE_OrderTraded,
        };

        enum
        {
            SSE_NoError,
            SSE_OrderReject_NotEnoughCash,
            SSE_OrderReject_NotEnoughInventory
        };

        uint32_t instrumentID;
        uint32_t orderQty;
        uint32_t filledQty;
        double orderprice;
        double tradeprice;
        bool   isbuy;
        uint8_t  orderStatus;
        uint8_t  orderErrorCode;
    };

    typedef SSE_OrderDetail SZE_OrderDetail;


    struct CFFEX_OrderDetail
    {

    };

    typedef CFFEX_OrderDetail  DCE_OrderDetail;
    typedef CFFEX_OrderDetail  SHFE_OrderDetail;
    typedef CFFEX_OrderDetail  CZCE_OrderDetail;


    //  the strategy send requestID with  sendID  
    struct OrderRequest
    {
        union
        {
                uint64_t requestID;
                SenderID senderID;
        } ;
        uint16_t requestType;

        union 
        {
            SSE_SecurityCancelRequest       sse_securityCancelRequest;
            SSE_SecurityModifyOrderRequest  sse_securityModifyOrderRequest;
            SSE_SecurityNewOrderRequest     sse_securityNewOrder;
        } ;

        void fillDetail(const SSE_SecurityCancelRequest& request) { sse_securityCancelRequest  = request;}
        void fillDetail(const SSE_SecurityModifyOrderRequest& request) { sse_securityModifyOrderRequest  = request;}
        void fillDetail(const SSE_SecurityNewOrderRequest& request) { sse_securityNewOrder  = request;}
    };

    struct OrderDataDetail
    {
        // indicate which request  send this order
        union
        {
            uint64_t requestid;
            SenderID senderid;
        };
        // other fields
        // normal it equal to requestID. but sometimes we may meet one request multi orders.
        uint64_t  orderID;
        uint16_t  exchangeType;

        union 
        {
            SSE_OrderDetail  sse_order;
            SZE_OrderDetail sze_order;
            CFFEX_OrderDetail cffex_order;
            DCE_OrderDetail dce_order;
            SHFE_OrderDetail shfe_order;
            CZCE_OrderDetail czce_order;
        };

        bool operator==(const OrderDataDetail& left)
        {
            return memcmp(&left, this, sizeof(OrderDataDetail)) == 0;
        }
    };
}