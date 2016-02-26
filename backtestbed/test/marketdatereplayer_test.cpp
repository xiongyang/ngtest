#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <sstream>
#include "../marketdatareplayer.h"
#include "../marketdatastore.h"
namespace BluesTrading
{
    using ::testing::Invoke;
    using ::testing::InvokeWithoutArgs;
    using ::testing::An;
    using ::testing::_;
    using ::testing::Field;
    using ::testing::Sequence;
    using ::testing::Truly;

    auto isDepthsLevel1 = [=](const CTickData& tick) 
    { 
        return tick.askLevels == 1;
    };

    auto isDepthsLevel5 = [=](const CTickData& tick) 
    { 
        return tick.askLevels == 5;
    };


    class FakeTickDataConsumer :public ITickDataConsumer
    {
    public:
        FakeTickDataConsumer() :current_time(0){}
        virtual void onMarketData(const CTickData& tick)
        {
            //EXPECT_EQ(1,tick.depthsNum);
            EXPECT_LE(current_time, tick.timeInMS);
            current_time = tick.timeInMS;
        };

        uint32_t current_time;
    };
    class MockTickDataConsumer : public ITickDataConsumer , public ITimerConsumer
    {
    public:

       //template<int N>
       //void registerDelegate()
       //{
       //    typedef  void  (FakeTickDataConsumer::* tickFun)(const CTickData<N>&);
       //    tickFun tifun =  &FakeTickDataConsumer::onMarketData;

       //    ON_CALL(*this,     onMarketData(An<const CTickData<N>&>()))
       //        .WillByDefault(Invoke(&fake, tifun));
       //}
        MockTickDataConsumer()
        {
            //registerDelegate<1>();
            //registerDelegate<5>();
            //registerDelegate<10>();
            //registerDelegate<20>();
            ON_CALL(*this, onMarketData(_))
                .WillByDefault(Invoke(&fake, &FakeTickDataConsumer::onMarketData));

            ON_CALL(*this, onEndDay(_)).WillByDefault(InvokeWithoutArgs(this, &MockTickDataConsumer::resetFakeTime));
        }


         MOCK_METHOD1(onMarketData, void(const CTickData&));
         //MOCK_METHOD1(onMarketData, void(const TickDataLevel5&));
         //MOCK_METHOD1(onMarketData, void(const TickDataLevel10&));
         //MOCK_METHOD1(onMarketData, void(const TickDataLevel20&));
         //MOCK_METHOD1(onOtherLevelsMarketData, void(const  CTickData<1>&));
         MOCK_METHOD1(onStartDay, void(uint32_t));
         MOCK_METHOD1(onEndDay, void(uint32_t));
         void onTimer(uint32_t eventID, uint32_t currentTime){}
         
         void resetFakeTime()
         {
             fake.current_time = 0;
         }
    private:
        FakeTickDataConsumer fake;
    };


    class marketdatareplayerTest : public ::testing::Test {
    protected:
        virtual void SetUp() 
        {
            replayer = NULL;
        }

       
        void generateFakeTick(uint32_t inst, uint32_t date,  uint32_t timeInMS_offset = 0,uint16_t levels = 1,uint32_t tick_num = 10)
        {
            MarketDataStore newStore(inst,date);
            //newStore.date = date;
            //newStore.instIndex = inst;
            for (uint32_t i = 0 ; i != tick_num; ++i)
            {
                CTickData tick;
                tick.askLevels = levels;
                tick.bidLevels = levels;
                tick.instIndex = inst;
                tick.timeInMS = i + timeInMS_offset;
                tick.depths.push_back(CTickData::Depth{0.1, i});
                tick.depths.push_back(CTickData::Depth{0.1, i});
                newStore.tickDataVec.push_back(std::move(tick));
            }

            fakedata.push_back(std::move(newStore));
        }

        void packDatatoReplayer()
        {
            if (replayer)  delete replayer;
            replayer = new MarketDataReplayer(fakedata);
        }

        virtual void TearDown() 
        {
            if(replayer) delete replayer;
             
        }

        std::vector<MarketDataStore> fakedata;
        MarketDataReplayer*  replayer;
    };


    TEST(testGetDate, getDateTest)
    {
        EXPECT_EQ(20131010, getDate("2013-10-10 09:17:13"));
    }

    TEST(testGetTime, getTimeTest)
    {
        uint32_t time = 9 * 3600 * 1000 + 17 * 60 * 1000 + 13 * 1000;
        EXPECT_EQ(time, getTime("2013-10-10 09:17:13"));
    }

    TEST_F(marketdatareplayerTest, marketstore_save_load_test)
    {
        const int tickNum = 20;
        generateFakeTick(1,20151230, 0 , 1,tickNum);

        std::stringstream ss;
        boost::archive::text_oarchive oa(ss);
        oa << fakedata[0];

 
        MarketDataStore inst;
        boost::archive::text_iarchive ia(ss);

        ia >> inst;

        EXPECT_EQ(inst.date, 20151230);
        EXPECT_EQ(inst.instIndex, 1);
        EXPECT_EQ(inst.tickDataVec.size() , tickNum);

        for(int i = 0; i != tickNum ; ++i)
        {
            EXPECT_EQ(inst.tickDataVec[i].timeInMS , fakedata[0].tickDataVec[i].timeInMS);
        }
    }

    TEST_F(marketdatareplayerTest, singleTickStore)
    {
        const int tickNum = 20;
        generateFakeTick(1,20151230, 0 , 1,tickNum);

        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);
        replayer->getTimerProvider()->registerTimerConsumer(&mockconsumer);
 
        Sequence s1;
        EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(An<const CTickData&>())).Times(tickNum).InSequence(s1);
        EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);


        replayer->startReplay(20151230, 20151231);  
    }

    TEST_F(marketdatareplayerTest, singleTickStoreWithSomeDoesNeedTick)
    {
        const int tickNum = 20;
        generateFakeTick(1, 20151230, 0 , 1, tickNum);
        generateFakeTick(2, 20151230, 0 , 1, tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);

        replayer->getTimerProvider()->registerTimerConsumer(&mockconsumer);

        using ::testing::_;
        using ::testing::An;
        using ::testing::Field;

      
        Sequence s1;
        EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(An<const CTickData&>())).Times(tickNum);
        EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);



        replayer->startReplay(20151230, 20151231);  
    }

    TEST_F(marketdatareplayerTest, multiTickStoreWithSomeDoesNeedTick)
    {
        const int tickNum = 20;
        generateFakeTick(1,20151230, 0 , 1,tickNum);
        generateFakeTick(1,20151230, 5 , 5, tickNum);
        generateFakeTick(2,20151230, 0 , 1, tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);
        replayer->getTimerProvider()->registerTimerConsumer(&mockconsumer);

        Sequence s1;
        EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(_)).Times(tickNum * 2).InSequence(s1);
        EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);



        replayer->startReplay(20151230, 20151231);  
    }


    TEST_F(marketdatareplayerTest, multiTickDifferentDepths)
    {
        const int tickNum = 20;
        generateFakeTick(1,20151230, 0 , 1,  tickNum);
        generateFakeTick(1,20151230, 5 , 5,  tickNum);
        generateFakeTick(2,20151230, 0 , 1,  tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);
        replayer->getTimerProvider()->registerTimerConsumer(&mockconsumer);

        using ::testing::_;
        using ::testing::An;
        using ::testing::Field;

        Sequence s1;
        EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);

        EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel1))).Times(tickNum);
        EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel5))).Times(tickNum);
        EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);
        replayer->startReplay(20151230, 20151231);  
    }

    TEST_F(marketdatareplayerTest, different_instrument_mixed)
    {
        const int tickNum = 20;
        generateFakeTick(1,20151230, 0 , 1, tickNum);
        generateFakeTick(1,20151230, 5 , 5, tickNum);
        generateFakeTick(2,20151230, 0 , 1, tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);
        replayer->subscribeInstrument(2, &mockconsumer);
        replayer->getTimerProvider()->registerTimerConsumer(&mockconsumer);

        using ::testing::_;
        using ::testing::An;
        using ::testing::Field;

        Sequence s1;


        auto isInstrument1 = [=](const CTickData& tick) 
        { 
            return tick.instIndex == 1;
        };

        auto isInstrument2 = [=](const CTickData& tick) 
        { 
            return tick.instIndex == 2;
        };

        EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(Truly(isInstrument1))).Times(tickNum * 2);
        EXPECT_CALL(mockconsumer, onMarketData(Truly(isInstrument2))).Times(tickNum);
        EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);
        replayer->startReplay(20151230, 20151231);  
    }

    TEST_F(marketdatareplayerTest, unsubscribe_instument)
    {
        const int tickNum = 20;
        generateFakeTick(1,20151230, 0 , 1, tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);
        replayer->getTimerProvider()->registerTimerConsumer(&mockconsumer);
      
        Sequence s1;
        EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel1))).Times(tickNum );
        EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);
        
        
        replayer->startReplay(20151230, 20151231);  

    
        Sequence s2;
        EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s2);
        //ensure the recall startReplay will play the tick  
        EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel1))).Times(tickNum);
        EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s2);
        mockconsumer.resetFakeTime();
        replayer->startReplay(20151230, 20151231);  
    }


    TEST_F(marketdatareplayerTest, date_range)
    {
        generateFakeTick(1,20151230, 0 , 1, 10);
        generateFakeTick(2,20151231, 0 , 1, 100);
        generateFakeTick(1,20151231, 0 , 1, 100);
        generateFakeTick(1,20160101, 0 , 1, 1000);
        generateFakeTick(1,20160102, 0 , 1, 10000);
        packDatatoReplayer();
       

        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);
        replayer->getTimerProvider()->registerTimerConsumer(&mockconsumer);
        {
            Sequence s1 ;

            EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
            EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel1))).Times(10).InSequence(s1);
            EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);


            EXPECT_CALL(mockconsumer, onStartDay(20151231)).InSequence(s1);
            EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel1))).Times(100).InSequence(s1);
            EXPECT_CALL(mockconsumer, onEndDay(20151231)).InSequence(s1);

            mockconsumer.resetFakeTime();
            replayer->startReplay(20151230, 20160101);  
        }


        

        {
            Sequence s1 ;
            EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
            EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel1))).Times(10).InSequence(s1);
            EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);
            mockconsumer.resetFakeTime();
            replayer->startReplay(20151230, 20151231);  
        }


        {
            Sequence s1 ;
            EXPECT_CALL(mockconsumer, onStartDay(20151231)).InSequence(s1);
            EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel1))).Times(100).InSequence(s1);
            EXPECT_CALL(mockconsumer, onEndDay(20151231)).InSequence(s1);


            EXPECT_CALL(mockconsumer, onStartDay(20160101)).InSequence(s1);
            EXPECT_CALL(mockconsumer, onMarketData(Truly(isDepthsLevel1))).Times(1000).InSequence(s1);
            EXPECT_CALL(mockconsumer, onEndDay(20160101)).InSequence(s1);
            mockconsumer.resetFakeTime();
            replayer->startReplay(20151231, 20160102); 
        }

    }

}
