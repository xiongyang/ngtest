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

        SZE_SecurityCancel ,
        SZE_SecurityNewOrder,
        SZE_SecurityModify,


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

    enum ExchangeTypes
    {
        Exch_CFFEX,
        Exch_DCE,
        Exch_SSE,
        Exch_SZE,
        Exch_SHFE,
        Exch_CZCE
    };

    enum OrderPriceType
    {
         OrderPriceType_None  = 0,
         OrderPriceType_LimitOrder ,
         OrderPriceType_MarketOrder,
    };

    enum OrderTimeType
    {
        OrderTimeType_None = 0,
        OrderTimeType_IOC,
    };

    // our client can send order via Close. but OrderManager may select the actually Type
    enum OpenCloseFlag
    {
        OpenCloseFlag_None = 0,
        OpenCloseFlag_Open,
        OpenCloseFlag_Close,
        OpenCloseFlag_CloseToday,
        OpenCloseFlag_CloseYst,
    };

    enum HedgeFlag
    {
        HedgeFlag_None = 0,
         HedgeFlag_Speculation,
         HedgeFlag_Arbitrage,
        //OrderHedgeFlag_Hedge,
        //OrderHedgeFlag_CFFEX_MarketMaker,
        //OrderHedgeFlag_Unknown
    };

    enum LongShortFlag
    {
        LongShortFlag_None = 0,
        LongShortFlag_Long ,
        LongShortFlag_Short,
    };


    enum  OrderStatus
    {
        OrderPendingNew = 0,
        OrderNew,
        OrderRejected,
        OrderCanceled,
        OrderFilled,
        OrderPartFilled,
    };

    enum OrderErrorCode
    {
         NoError,
         OrderReject_NotEnoughCash,
         OrderReject_NotEnoughInventory
    };



    struct OrderUpdateMask
    {
        uint8_t marketID;
        uint8_t accountID;
        uint8_t machineID;
        uint8_t strategyID;
    };

    struct SenderID
    {
        union
        {
            uint32_t    updateMaskID;
            OrderUpdateMask updateMask;
        };

        uint32_t requestID;
    };

    template<int RequestTypeValue>
    struct CancelRequest
    {
        static const int RequestType = SSE_SecurityCancel;
        uint64_t blueOrderID;
    };

    typedef CancelRequest<SSE_SecurityCancel> SSE_SecurityCancelRequest;
    typedef CancelRequest<SZE_SecurityCancel> SZE_SecurityCancelRequest;
    typedef CancelRequest<CFFEX_IndexFutureCancel> CFFEX_CancelRequest;
    typedef CancelRequest<DCE_ProductFutureCancel> DCE_CancelRequest;
    typedef CancelRequest<SHFE_ProductFutureCancel> SHFE_CancelRequest;
    typedef CancelRequest<CZCE_ProductFutureCancel> CZCE_CancelRequest;

    template<int RequestTypeValue>
    struct SecurityNewOrderRequest
    {
        static const int RequestType = RequestTypeValue;
        uint32_t instrumentID;
        double price;
        uint32_t orderqty; 

        LongShortFlag longshortflag;

    };

    typedef SecurityNewOrderRequest<SSE_SecurityNewOrder> SSE_SecurityNewOrderRequest;
    typedef SecurityNewOrderRequest<SZE_SecurityNewOrder> SZE_SecurityNewOrderRequest;

    template<int RequestTypeValue>
    struct  SecurityModifyOrderRequest
    {
        static const int RequestType = RequestTypeValue;
        SecurityNewOrderRequest<RequestTypeValue>   newOrder;
        uint64_t blueOrderID;   
    };

    typedef SecurityModifyOrderRequest<SSE_SecurityModify> SSE_SecurityModifyOrderRequest;
    typedef SecurityModifyOrderRequest<SZE_SecurityModify> SZE_SecurityModifyOrderRequest;

    struct CommonOrderStatusDetail
    {
        OrderStatus  orderStatus;
        OrderErrorCode  orderErrorCode;
        //micro time from epoch

        uint64_t    submitTime;
        uint64_t    updateTime;
    };

    struct SecurityOrderDetail
    {
        uint32_t instrumentID;
        uint32_t orderQty;
        uint32_t filledQty;
        double orderprice;
        double tradeprice;
        LongShortFlag   longshortflag;


        CommonOrderStatusDetail common;
    };

    typedef SecurityOrderDetail SSE_OrderDetail;
    typedef SecurityOrderDetail SZE_OrderDetail;

    template<int RequestTypeValue>
    struct Future_NewOrderRequest
    {
        //Future_NewOrderRequest()
        //{
        //    memset(this, 0, sizeof(Future_NewOrderRequest<RequestTypeValue>));
        //}


        static const int RequestType = RequestTypeValue;
        uint32_t instrumentID;
        uint32_t orderqty; 
        double price;
        

        OpenCloseFlag opencloseflag;
        OrderPriceType pricetype;
        OrderTimeType timetype;
        HedgeFlag hedgeflag;
        LongShortFlag longshortflag;
    };


    typedef Future_NewOrderRequest<CFFEX_IndexFutureNewOrder>  CFFEX_NewOrderRequest;
    typedef Future_NewOrderRequest<DCE_ProductFutureNewOrder>  DCE_NewOrderRequest;
    typedef Future_NewOrderRequest<SHFE_ProductFutureNewOrder>  SHFE_NewOrderRequest;
    typedef Future_NewOrderRequest<CZCE_ProductFutureNewOrder>  CZCE_NewOrderRequest;

    template<int RequestTypeValue>
    struct Future_ModifyOrderRequest
    {
        static const int RequestType = RequestTypeValue;
        Future_NewOrderRequest<RequestTypeValue>  newOrder;
        uint64_t blueOrderID;   
    };

    typedef Future_ModifyOrderRequest<CFFEX_IndexFutureModify>  CFFEX_ModifyOrderRequest;
    typedef Future_ModifyOrderRequest<DCE_ProductFutureModify>  DCE_ModifyOrderRequest;
    typedef Future_ModifyOrderRequest<SHFE_ProductFutureModify>  SHFE_ModifyOrderRequest;
    typedef Future_ModifyOrderRequest<CZCE_ProductFutureModify>  CZCE_ModifyOrderRequest;

    struct FutureOrderDetail
    {
        uint32_t instrumentID;
        uint32_t orderqty; 
        double price;
        double tradeprice ;
        uint32_t filledQty;

        OpenCloseFlag opencloseflag;
        OrderPriceType pricetype;
        OrderTimeType timetype;
        HedgeFlag hedgeflag;
        LongShortFlag longshortflag;
        CommonOrderStatusDetail common;
    };

    typedef FutureOrderDetail CFFEX_OrderDetail;
    typedef FutureOrderDetail DCE_OrderDetail;
    typedef FutureOrderDetail SHFE_OrderDetail;
    typedef FutureOrderDetail CZCE_OrderDetail;


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

            SZE_SecurityCancelRequest       sze_securityCancelRequest;
            SZE_SecurityModifyOrderRequest  sze_securityModifyOrderRequest;
            SZE_SecurityNewOrderRequest     sze_securityNewOrder;

            CFFEX_NewOrderRequest cffex_neworderrequest;
            DCE_NewOrderRequest dce_neworderrequest;
            SHFE_NewOrderRequest shfe_neworderrequest;
            CZCE_NewOrderRequest czce_neworderrequest;

            CFFEX_ModifyOrderRequest cffex_modifyorderrequest;
            DCE_ModifyOrderRequest dce_modifyorderrequest;
            SHFE_ModifyOrderRequest shfe_modifyorderrequest;
            CZCE_ModifyOrderRequest czce_modifyorderrequest;

            CFFEX_CancelRequest cffex_cancelrequest;
            DCE_CancelRequest dce_cancelrequest;
            SHFE_CancelRequest shfe_cancelrequest;
            CZCE_CancelRequest czce_cancelrequest;
        } ;

        void fillDetail(const SSE_SecurityCancelRequest& request) { sse_securityCancelRequest  = request;}
        void fillDetail(const SSE_SecurityModifyOrderRequest& request) { sse_securityModifyOrderRequest  = request;}
        void fillDetail(const SSE_SecurityNewOrderRequest& request) { sse_securityNewOrder  = request;}

        void fillDetail(const SZE_SecurityCancelRequest& request) { sze_securityCancelRequest  = request;}
        void fillDetail(const SZE_SecurityModifyOrderRequest& request) { sze_securityModifyOrderRequest  = request;}
        void fillDetail(const SZE_SecurityNewOrderRequest& request) { sze_securityNewOrder  = request;}

        void fillDetail(const CFFEX_NewOrderRequest& request) { cffex_neworderrequest  = request;}
        void fillDetail(const DCE_NewOrderRequest& request) { dce_neworderrequest  = request;}
        void fillDetail(const SHFE_NewOrderRequest& request) { shfe_neworderrequest  = request;}
        void fillDetail(const CZCE_NewOrderRequest& request) { czce_neworderrequest  = request;}

        void fillDetail(const CFFEX_ModifyOrderRequest& request) { cffex_modifyorderrequest  = request;}
        void fillDetail(const DCE_ModifyOrderRequest& request) { dce_modifyorderrequest  = request;}
        void fillDetail(const SHFE_ModifyOrderRequest& request) { shfe_modifyorderrequest  = request;}
        void fillDetail(const CZCE_ModifyOrderRequest& request) { czce_modifyorderrequest  = request;}


        void fillDetail(const CFFEX_CancelRequest& request) { cffex_cancelrequest  = request;}
        void fillDetail(const DCE_CancelRequest& request) { dce_cancelrequest  = request;}
        void fillDetail(const SHFE_CancelRequest& request) { shfe_cancelrequest  = request;}
        void fillDetail(const CZCE_CancelRequest& request) { czce_cancelrequest  = request;}

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
        // blueOrderID
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

    

    //struct SecurityTrade
    //{
    //    uint32_t instrumentID;
    //    double tradeprice;
    //    uint32_t tradeqty;
    //  
    //    uint64_t tradeTime;
    //    LongShortFlag   longshortflag;
    //};

    //struct FutureTrade
    //{
    //    uint32_t instrumentID;
    //    double tradeprice;
    //    uint32_t tradeqty;
    //    LongShortFlag   longshortflag;
    //    OpenCloseFlag opencloseflag;
    //    HedgeFlag       hedgeflag;
    //};

    //struct TradeDataDetail
    //{
    //    union
    //    {
    //        uint64_t requestid;
    //        SenderID senderid;
    //    };

    //    uint64_t  orderID;
    //    uint16_t  exchangeType;

    //    union 
    //    {
    //        SecurityTrade  security_trade;
    //        FutureTrade future_trade;
    //    };
    //};
}