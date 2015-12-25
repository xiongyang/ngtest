#pragma  once
#include <cstdint>

namespace BluesTrading
{
	class ITimerConsumer
	{
		virtual void onTimer(uint32_t eventID, uint32_t currentTime) = 0;
	};

	class ITimerProvider
	{
	public:
		virtual void setTimer(uint32_t eventID, uint32_t timeInMS, bool repeat) = 0;
		virtual uint32_t getCurrentTimeInDay() = 0;  // MS in Day
		virtual uint32_t getCurrentDate() = 0;  // YYYYMMDD
	};

}
