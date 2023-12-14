/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/
#pragma once
#include <chrono>
#include <stdio.h>
#include <cstdint>
#include <limits>
#include <sstream>
#include <iomanip>

#include "dispatch/core/Logger/Logger.hh"

namespace claid
{
struct Duration 
{

	bool valid;
	std::chrono::microseconds val;

	Duration() : valid(false)
	{

	}

	Duration(std::chrono::microseconds microseconds) : valid(true), val(microseconds)
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

	static Duration days(int days)
	{
		std::chrono::hours d(days * 24);
		return Duration(std::chrono::duration_cast<std::chrono::microseconds>(d));
	}

	static Duration hours(int hours)
	{
		std::chrono::hours h(hours);
		return Duration(std::chrono::duration_cast<std::chrono::microseconds>(h));
	}

	static Duration minutes(int minutes)
	{
		std::chrono::minutes m(minutes);
		return Duration(std::chrono::duration_cast<std::chrono::microseconds>(m));
	}

	bool operator==(const Duration& other) const
	{
		return this->getNanoSeconds() == other.getNanoSeconds();
	}

	bool operator<(const Duration& other) const 
	{
		return this->getNanoSeconds() < other.getNanoSeconds();
	}

	bool operator>(const Duration& other) const 
	{
		return this->getNanoSeconds() > other.getNanoSeconds();
	}

	bool operator>=(const Duration& other) const 
	{
		return this->getNanoSeconds() >= other.getNanoSeconds();
	}
};



class Time : public std::chrono::time_point<std::chrono::system_clock>
{

private:
    typedef std::chrono::time_point<std::chrono::system_clock>  Base;
	bool valid = false;

public:


    Time() : valid(true)
    { 

    }

	Time(bool valid) : valid(valid)
	{

	}

    Time(std::chrono::time_point<std::chrono::system_clock> timePoint) : Base(timePoint), valid(true)
    {
		
    }

	static Time invalidTime()
	{
		return Time(false);
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
     
	bool isValid() const
	{
		return this->valid;
	}

public:


		Duration subtract(const Time& t)
		{
			return Duration(std::chrono::milliseconds(this->toUnixTimestampMilliseconds() - t.toUnixTimestampMilliseconds()));
		}

		Time operator+(const Duration& d) const
		{
			Time a = *this;
			a += std::chrono::microseconds(d.val);

			return a;
		}

		Time& operator+=(const Duration& d) 
		{
			return (Time&) Base::operator+=(d.val);
		}
		
		Time operator-(const Duration& d) const
		{
			Time a = *this;
			a -= std::chrono::microseconds(d.val);

			return a;
		}

		Time& operator-=(const Duration& d) 
		{
			return (Time&) Base::operator-=(d.val);
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


	};
}




