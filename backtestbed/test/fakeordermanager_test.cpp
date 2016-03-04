#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../fakeordermanager.h"
#include "../faketimerprovider.h"
#include "util.h"

namespace BluesTrading
{
    using ::testing::Invoke;
    using ::testing::InvokeWithoutArgs;
    using ::testing::An;
    using ::testing::_;
    using ::testing::Field;
    using ::testing::Sequence;
    using ::testing::Truly;

    class MockOrderConsumer: public IOrderDataConsumer
    {
    public:
        MOCK_METHOD1(onUpdateOrder,  void(OrderDataDetail* orderData));
    };


    class FakeOrderManagerTest : public ::testing::Test {
    protected:
        virtual void SetUp() 
        {
            inst.subscribeOrderUpdate(1,&consumer);
        }

        virtual void TearDown() 
        {
        }
        MockOrderConsumer consumer;
        FakeOrderManager inst;
    };

    TEST_F(FakeOrderManagerTest, tradeExpect)
    {
        SSE_SecurityNewOrderRequest request;
        request.instrumentID = 1;
        request.isBuy = true;
        request.orderqty = 100;
        request.orderType = 0; //only limit order
        request.price = 10.1;
        request.priceType = 0; // only limit order


        auto isfirstOrder = [=](OrderDataDetail* porder) 
        { 
            printOrder(std::cout , *porder);
            SSE_OrderDetail& sse_order =  porder->sse_order;
            EXPECT_EQ(sse_order.instrumentID , 1);
            EXPECT_EQ(sse_order.isbuy , true);
            EXPECT_EQ(sse_order.orderQty , 100);

            EXPECT_DOUBLE_EQ(10.1,sse_order.orderprice);

            if(sse_order.filledQty == 0)
            {
                EXPECT_DOUBLE_EQ(0 , sse_order.tradeprice);
                return true;
            }
            else
            {
                return false;
            }
        };

        EXPECT_CALL(consumer, onUpdateOrder(Truly(isfirstOrder))).Times(1);

        auto istradeOrder = [=](OrderDataDetail* porder) 
        {
            printOrder(std::cout , *porder);
            SSE_OrderDetail& sse_order =  porder->sse_order;
            EXPECT_EQ(sse_order.instrumentID , 1);
            EXPECT_EQ(sse_order.isbuy , true);
            EXPECT_EQ(sse_order.orderQty , 100);
            EXPECT_DOUBLE_EQ(10.1 , sse_order.orderprice);

            if(sse_order.filledQty == 0)
            {
                return false;
            }
            else
            {
                EXPECT_DOUBLE_EQ(10.1 , sse_order.tradeprice);
                return true;
            }
        };


        EXPECT_CALL(consumer, onUpdateOrder(Truly(istradeOrder))).Times(1);
        uint64_t requestID = submitRequest(request, &inst);
    }
}