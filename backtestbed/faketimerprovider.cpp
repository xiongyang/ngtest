#include "faketimerprovider.h"
#include "util.h"

#include<iostream>

//
//#include <windows.h>
//#include <DbgHelp.h>
//#include <WinBase.h>
//
//#pragma comment(lib, "dbghelp.lib")
//
//
//void printStack( void );
//void printStack( void )
//{
//    unsigned int   i;
//    void         * stack[ 100 ];
//    unsigned short frames;
//    SYMBOL_INFO  * symbol;
//    HANDLE         process;
//
//    process = GetCurrentProcess();
//
//    SymInitialize( process, NULL, TRUE );
//
//    frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
//    symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
//    symbol->MaxNameLen   = 255;
//    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
//
//    for( i = 0; i < frames; i++ )
//    {
//        SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );
//
//        printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
//    }
//
//    free( symbol );
//}
//
////void printStack(void)
////{
////    char cBuff[1024] = {0};
////    unsigned int   i;
////    void         * stack[128];
////    unsigned short frames;
////    SYMBOL_INFO  * symbol;
////    HANDLE         process;
////
////    process = GetCurrentProcess();
////
////    SymInitialize(process, NULL, TRUE);
////
////    frames = CaptureStackBackTrace(0, 128, stack, NULL);
////    symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
////    symbol->MaxNameLen = 255;
////    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
////
////    OutputDebugString(_T("##########################################################################\n"));
////    for (i = 0; i < frames; i++)
////    {
////        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
////        sprintf_s(cBuff,1024,("%i: %s - 0x%0X\n"),frames - i - 1, symbol->Name, symbol->Address);
////        OutputDebugStringA(cBuff);
////    }
////    OutputDebugString(_T("--------------------------------------------------------------------\n"));
////
////    free(symbol);
////}

namespace BluesTrading
{
    void printTimer(FakeTimerProvider::TimerInfo* next_timer)
    {
        if (next_timer == NULL)
        {
            std::cout << "PrintTimer Null " << std::endl;
            return;
        }
        std::cout << "PrintTimer  " << next_timer << " Consumer " << next_timer->consumer << " EventID:"<< next_timer->eventid 
            << " TimeInterVal:" << next_timer->interval << " NextTriggerTime:" << next_timer->nextTriggerTime << " IsRepeat " << next_timer->isRepeat << std::endl;
    }


    bool FakeTimerProvider::setTimer(ITimerConsumer* consumer, uint32_t eventID, uint32_t timeInMSFromNow, bool repeat)
    {
        uint32_t triggerTime = timeInMSFromNow + current_time;
        TimerInfo* info =  new TimerInfo
        {
                consumer,
                eventID,
                triggerTime,
                timeInMSFromNow,
                repeat
        };
        bool ret = allTimers[consumer].insert(std::make_pair(eventID, info)).second;
        std::cout << "Insert New Timer at Mem " << allTimers[consumer][eventID] << std::endl;
        printTimer(info);
        return ret;
    }

    bool FakeTimerProvider::cancelTimer(ITimerConsumer* consumer, uint32_t eventID)
    {
      return  allTimers[consumer].erase(eventID) == 1;
    }


    std::uint32_t FakeTimerProvider::getCurrentTimeMsInDay()
    {
        return current_time;
    }

    std::uint32_t FakeTimerProvider::getCurrentDate()
    {
        return current_date;
    }

    void FakeTimerProvider::setNextTickTime(uint32_t timeInMs)
    {
       advanceToTime(timeInMs);
    }



    void FakeTimerProvider::setDate(uint32_t a_date)
    {
        if (a_date != current_date)
        {
            invokeAllTimerOnce();
            current_date = a_date;
        }
    }

    void FakeTimerProvider::invokeAllTimerOnce()
    {

        for(auto eachConsumerMap : allTimers)
        {
            for(auto eachTimer : eachConsumerMap.second)
            {
                eachTimer.second->consumer->onTimer(eachTimer.second->eventid,eachTimer.second->nextTriggerTime);
            }
        }
    }

    void FakeTimerProvider::advanceToTime(uint32_t timeInMs)
    {
       static int count = 0;
       TimerInfo*  next_timer = getNextTimer();
       

       ScopeGuard  onExit ([&]{current_time = timeInMs;  std::cout << "advanceToTime  " << timeInMs << "\n"; });


        while (next_timer != NULL && next_timer->nextTriggerTime <= timeInMs)
        {
            current_time = next_timer->nextTriggerTime ;
          
            printTimer(next_timer);
            next_timer->consumer->onTimer(next_timer->eventid, current_time);
            if (next_timer->isRepeat)
            {
                next_timer->nextTriggerTime += next_timer->interval;
            }
            else
            {
                cancelTimer(next_timer->consumer,next_timer->eventid);
            }
           next_timer = getNextTimer();
        }
    }

    FakeTimerProvider::TimerInfo* FakeTimerProvider::getNextTimer()
    {
        TimerInfo* ret = NULL;

        for(auto eachConsumerMap : allTimers)
        {
            for(auto eachTimer : eachConsumerMap.second)
            {
               if (ret == NULL)
               {
                   ret = eachTimer.second;
                   continue;
               }

               if (ret->nextTriggerTime > eachTimer.second->nextTriggerTime)
               {
                   ret = eachTimer.second;
               }
            }
        }

        return ret;
    } 

}

