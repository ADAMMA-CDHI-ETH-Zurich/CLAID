#pragma once
#include <chrono>
#include <stdio.h>
#include <cstdint>
#include <limits>
#include <sstream>
#include <iomanip>

namespace claid
{
struct Duration 
{

	bool valid;
	std::chrono::microseconds val;

	Duration() : valid(false)
	{

	}

	Duration(std::chrono::microseconds microseconds) : val(microseconds), valid(true)
	{
	}

	// bool isValid() const
	// {
	// 	// TODO
	// 	return true;
	// }

	// bool isInfinity() const
	// {
	// 	// TODO
	// 	return false;
	// }

	uint64_t getNanoSeconds() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(this->val).count();
	}


	uint64_t getMicroSeconds() const
	{
		return val.count();
	}

	uint64_t getMilliSeconds() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(this->val).count();
	}

	uint64_t getSeconds() const
	{
		return std::chrono::duration_cast<std::chrono::seconds>(this->val).count();
	}

	static Duration infinity()
	{
		std::chrono::milliseconds inf(std::numeric_limits<int>::max());
		return Duration(inf);
	}

	static Duration nanoseconds(int nanoseconds)
	{
		std::chrono::nanoseconds ns(nanoseconds);
		
		return Duration(std::chrono::duration_cast<std::chrono::microseconds>(ns));
	}

	static Duration microseconds(int microseconds)
	{
		std::chrono::microseconds mus(microseconds);
		
		return Duration(std::chrono::duration_cast<std::chrono::microseconds>(mus));
	}

	static Duration milliseconds(int milliseconds)
	{
		std::chrono::milliseconds ms(milliseconds);
		
		return Duration(std::chrono::duration_cast<std::chrono::microseconds>(ms));
	}

	static Duration seconds(int seconds)
	{
		std::chrono::seconds s(seconds);
		return Duration(std::chrono::duration_cast<std::chrono::microseconds>(s));
	}
};

#include "Reflection/SplitReflectInType.hpp"


class Time : public std::chrono::time_point<std::chrono::system_clock>
{

private:
    typedef std::chrono::time_point<std::chrono::system_clock>  Base;

public:
    std::chrono::time_point<std::chrono::system_clock> timePoint;


    Time()
    {

    }

    Time(std::chrono::time_point<std::chrono::system_clock> timePoint) : Base(timePoint), timePoint(timePoint)
    {
		
    }


	static Time now()
	{
		return std::chrono::system_clock::now();
	}

	static Time eternity()
	{
		return std::chrono::time_point<std::chrono::system_clock>::max();
	}

	static Time currentDay()
	{
		return todayAt(0, 0, 0);
	}

	static Time currentHour()
	{
		int year, month, day, hour, minute, second;
		getCurrentDateTime(year, month, day, hour, minute, second);
		return todayAt(hour, 0, 0);
	}

	static Time currentMinute()
	{
		int year, month, day, hour, minute, second;
		getCurrentDateTime(year, month, day, hour, minute, second);
		return todayAt(hour, minute, 0);
	}

	static Time currentSecond()
	{
		int year, month, day, hour, minute, second;
		getCurrentDateTime(year, month, day, hour, minute, second);
		return todayAt(hour, minute, second);
	}

	static Time today()
	{
		int year, month, day, hour, minute, second;
		getCurrentDateTime(year, month, day, hour, minute, second);
		int amount = 1000 * (hour * 60 * 60 + minute * 60 + second);

		return std::chrono::system_clock::now() - std::chrono::milliseconds(amount);
	}

	static Time todayAt(int hour, int minute, int second)
	{
		int amount = 1000 * (hour * 60 * 60 + minute * 60 + second);
		return today() + std::chrono::milliseconds(amount);
	}


	static Time unixEpoch()
	{
		const auto p0 = std::chrono::time_point<std::chrono::system_clock>();
		return p0;
	}

	uint64_t toUnixTimestamp() const
	{
		return std::chrono::duration_cast<std::chrono::seconds>(*this - Time::unixEpoch()).count();
	}

	uint64_t toUnixTimestampMilliseconds() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(*this - Time::unixEpoch()).count();
	}


	uint64_t toUnixNanoseconds() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(*this - Time::unixEpoch()).count();
	}

	
	static Time fromUnixTimestamp(uint64_t seconds)
	{
		return Time::unixEpoch() + std::chrono::seconds(seconds);
	}


	static Time fromUnixTimestampMilliseconds(uint64_t milliSeconds)
	{
		return Time::unixEpoch() + std::chrono::milliseconds(milliSeconds);
	}

	void toDateTime(int& year, int& month, int& day, int& hour, int& minute, int& second)
	{
		Time::toDateTime(*this, year, month, day, hour, minute, second);
	}

	static void toDateTime(Time& time, int& year, int& month, int& day, int& hour, int& minute, int& second)
	{
		time_t tt = std::chrono::system_clock::to_time_t(time);
		tm local_tm = *localtime(&tt);

		year = local_tm.tm_year + 1900;
		month = local_tm.tm_mon + 1;
		day = local_tm.tm_mday;
		hour = local_tm.tm_hour;
		minute = local_tm.tm_min;
		second = local_tm.tm_sec;
	}

	static void getCurrentDateTime(int& year, int& month, int& day, int& hour, int& minute, int& second)
	{
		Time now = Time::now();
		Time::toDateTime(now, year, month, day, hour, minute, second);
	}

	std::string strftime(const char* fmt) const
	{
		time_t tt = std::chrono::system_clock::to_time_t(*this);
		tm local_tm = *localtime(&tt);
		
		std::stringstream ss;
		ss << std::put_time(&local_tm, fmt);
		return ss.str();
	}
     

public:

		/** @name Operators */
		//@{
		std::chrono::duration<int64_t, std::nano> operator-(const Time& t) const
		{
			return this->timePoint - t.timePoint;
		}

		Time operator+(const Duration& d) const
		{
			return this->timePoint + d.val;
		}
		
		Time operator-(const Duration& d) const
		{
			return this->timePoint - d.val;
		}

		bool operator==(const Time& other) const
		{
			return this->toUnixNanoseconds() == other.toUnixNanoseconds();
		}

		bool operator<(const Time& other) const 
		{
			return this->toUnixNanoseconds() < other.toUnixNanoseconds();
		}

		bool operator>(const Time& other) const 
		{
			return this->toUnixNanoseconds() > other.toUnixNanoseconds();
		}

		bool operator>=(const Time& other) const 
		{
			return this->toUnixNanoseconds() >= other.toUnixNanoseconds();
		}

		template<typename Reflector>
		void reflect(Reflector& r)
		{
			splitReflectInType(r, *this);
		}

		template<typename Reflector>
		void reflectRead(Reflector& r)
		{
			uint64_t unixMs;
			r.member("UnixTimestamp", unixMs, "");

			*this = Time::fromUnixTimestampMilliseconds(unixMs);
		}

		template<typename Reflector>
		void reflectWrite(Reflector& r)
		{
			uint64_t unixMs = this->toUnixTimestampMilliseconds();
			r.member("UnixTimestamp", unixMs, "");
		}


	};
}




