#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../faketimerprovider.h"

namespace BluesTrading
{
    using ::testing::Invoke;
    using ::testing::InvokeWithoutArgs;
    using ::testing::An;
    using ::testing::_;
    using ::testing::Field;
    using ::testing::Sequence;

    class MockTimerConsumer: public ITimerConsumer
    {
    public:
        MOCK_METHOD2(onTimer,  void(uint32_t eventID, uint32_t currentTime));
    };


    class FakeTimerProviderTest : public ::testing::Test {
    protected:
        virtual void SetUp() 
        {
               timerProvider.setNextTickTime(startUpTimerInMs);
        }
       
        virtual void TearDown() 
        {
        }
        static const int startUpTimerInMs = 10000;
        FakeTimerProvider timerProvider;
    };

    TEST_F(FakeTimerProviderTest, singleConsumerSingleStepNoRepeat)
    {
        MockTimerConsumer inst;
        timerProvider.setTimer(&inst, 1, 1000, false);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 1000)).Times(1);
        timerProvider.setNextTickTime(10000 + 1001);
    }

    TEST_F(FakeTimerProviderTest, singleConsumerSingleStepNoRepeatMoreTime)
    {
        MockTimerConsumer inst;
        timerProvider.setTimer(&inst, 1, 1000, false);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 1000)).Times(1);
        timerProvider.setNextTickTime(startUpTimerInMs + 2001);

        EXPECT_EQ(startUpTimerInMs + 2001, timerProvider.getCurrentTimeMsInDay());
    }



    TEST_F(FakeTimerProviderTest, singleConsumerRepeat)
    {
        MockTimerConsumer inst;

        timerProvider.setTimer(&inst, 1, 1000, true);

        int repeatTimes = 10;
        


        Sequence s1;

        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 1000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 2000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 3000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 4000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 5000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 6000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 7000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 8000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 9000)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 10000)).Times(1).InSequence(s1);
        timerProvider.setNextTickTime(10000 + 1001 *  repeatTimes);
    }

    TEST_F(FakeTimerProviderTest, MultiConsumerNoRepeat)
    {
        MockTimerConsumer inst;
        MockTimerConsumer inst2;

        timerProvider.setTimer(&inst, 1, 1000, false);
        timerProvider.setTimer(&inst2, 1, 1000, false);


        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 1000 )).Times(1);
        EXPECT_CALL(inst2, onTimer(1,startUpTimerInMs  + 1000)).Times(1);

        timerProvider.setNextTickTime(startUpTimerInMs + 1500);
    }

    TEST_F(FakeTimerProviderTest, MultiConsumerNoRepeatDifferenetRegisterTime)
    {
        MockTimerConsumer inst;
        MockTimerConsumer inst2;

        timerProvider.setTimer(&inst, 1, 1000, false);
        timerProvider.setNextTickTime(startUpTimerInMs + 500);
        timerProvider.setTimer(&inst2, 2, 1000, false);

         Sequence s1;
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 1000 )).Times(1).InSequence(s1);
        EXPECT_CALL(inst2, onTimer(2,startUpTimerInMs  + 1500)).Times(1).InSequence(s1);

        timerProvider.setNextTickTime(startUpTimerInMs + 1600);
    }

    TEST_F(FakeTimerProviderTest, MultiConsumerNoRepeatDifferenetRegisterTime_oneRepeat)
    {
        MockTimerConsumer inst;
        MockTimerConsumer inst2;

        timerProvider.setTimer(&inst, 1, 1000, true);
        timerProvider.setNextTickTime(startUpTimerInMs + 500);
        timerProvider.setTimer(&inst2, 1, 1000, false);

        Sequence s1;
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 1000 )).Times(1).InSequence(s1);
        EXPECT_CALL(inst2, onTimer(1,startUpTimerInMs  + 1500)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 2000 )).Times(1).InSequence(s1);

        timerProvider.setNextTickTime(startUpTimerInMs + 2500);
    }

    TEST_F(FakeTimerProviderTest, MultiConsumerNoRepeatDifferenetRegisterTime_bothrepeat)
    {
        MockTimerConsumer inst;
        MockTimerConsumer inst2;

        timerProvider.setTimer(&inst, 1, 1000, true);
        timerProvider.setNextTickTime(startUpTimerInMs + 500);
        timerProvider.setTimer(&inst2, 1, 1000, true);

        Sequence s1;
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 1000 )).Times(1).InSequence(s1);
        EXPECT_CALL(inst2, onTimer(1,startUpTimerInMs  + 1500)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 2000 )).Times(1).InSequence(s1);
        EXPECT_CALL(inst2, onTimer(1,startUpTimerInMs  + 2500)).Times(1).InSequence(s1);
        EXPECT_CALL(inst, onTimer(1,startUpTimerInMs  + 3000 )).Times(1).InSequence(s1);

        timerProvider.setNextTickTime(startUpTimerInMs + 3000);

        EXPECT_CALL(inst2, onTimer(1,startUpTimerInMs  + 3500)).Times(1).InSequence(s1);
        timerProvider.setNextTickTime(startUpTimerInMs + 3500);

      
    }


}
