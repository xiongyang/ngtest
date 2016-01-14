#pragma once

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

    enum OrderStatus
    {
        PendingNew,
        New,
        Parti
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
        uin blueOrderID;
    };

    struct  SSE_SecurityModifyOrderRequest
    {
        static const int RequestType = SSE_SecurityModify;
        uint32_t instrumentID;
        uint8_t orderType;
        uint8_t priceType;
        uint8_t diretion;
        uint32_t price;
        uint32_t Orderqty; 
    };

    struct SSE_SecurityNewOrderRequest
    {
        static const int RequestType = SSE_SecurityNewOrder;
        uint32_t instrumentID;
        uint8_t orderType;
        uint8_t priceType;
        uint8_t direction;
        uint32_t price;
        uint32_t Orderqty; 
    };


    //  the stratgy send requestID with  sendID  
    struct OrderRequest
    {
        union
        {
                uint64_t requestID;
                SenderID senderID;
        }
        uint16_t requestType;

    public:

    private:

        union 
        {
            SSE_SecurityCancelRequest       sse_securityCancelRequest;
            SSE_SecurityModifyOrderRequest  sse_securityModifyOrderRequest;
            SSE_SecurityNewOrderRequest     sse_securityNewOrder;
        }
    };

    struct OrderDataDetail
    {
        uint16_t senderMachineID;
        uint16_t sendStrategyID;
        uint32_t requestID;
        // other fields
        uint32_t instrumentID;
        int32_t  orderID;
        uint32_t orderQty;
        uint32_t filledQty;
        double orderprice;
        double tradeprice;




        //int strategyID;
        //unsigned int instrumentID;
        //int orderID;//filled by OMS and used internally
        //char internalRefID; // to distinguish between two SOM's for the same strategy
        //int unitMultiplier;
        //FTDCOrderOffsetFlag offsetFlag;
        //OrderType orderType;
        //OrderType origOrderType;
        //OrderPriceType orderPriceType;
        //int orderQty; //int tradeQty;
        //double limitPrice;
        //OrderStatusType orderStatus;
        //unsigned int submitTick;
        //unsigned int updateTick;

        //int filledQtyAtCancel;
        //int filledQty;
        //int getFreeQty() const {
        //    return orderQty - filledQty;
        //}
        //double costBasis;//for those filled
        //double avgTradePrice() const {
        //    return filledQty == 0 ? 0 : costBasis / filledQty / unitMultiplier;
        //}

        ////COID
        //int exchangeID;//this is the mapped internal id, only for this session
        //int exchangeOrderID;//childOrder id at exchange side	

        //double tradePrice;
        //ChildOrderStatusType childOrderStatus;
        //DetailOrderStatusType detailStatus;

        //unsigned int oeeSubmitTickInUSec;

        //bool isToBeCancelled;
        //bool isParked;
        //double averagePrice() const {
        //    return filledQty == 0 ? 0 : costBasis / filledQty;
        //}

        //bool dataValidFlag;
        //bool isNonIOC;

        //bool isCancelSubmitted:1;
        //bool isExchangeAckReceived:1;

        //char hedgeFlag;
        //unsigned short x64pad1;//pad it to 108+4 = 112 %8 == 0 for x64 memcpy efficiency
    };
}