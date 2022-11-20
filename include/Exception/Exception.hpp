#pragma once
#include <assert.h>

#include <string.h>

#include <exception>
#include <sstream>
#include <iostream>

#include <list>
#include <chrono>
#include <thread>
#include "Logger/Logger.hpp"


#define CLAID_THROW(ex, msg)\
{\
	std::ostringstream ex_str;\
	ex_str << msg;\
    claid::Logger::printfln("%s", ex_str.str().c_str());\
	throw ex(ex_str.str(), __FILE__, __LINE__); \
}
// std::this_thread::sleep_for(std::chrono::milliseconds(1000));

namespace claid
{
	class Exception : public std::exception
	{

	protected:
		Exception()  noexcept {}

	public:


		Exception(const std::string& message, const char* file=NULL, int line=0) 
		{
			addInfo(message, file, line);
		}

		virtual ~Exception() noexcept {}

		void addInfo(const std::string& message, const char* file=NULL, int line=0)
		{
			mInfos.push_back(Info(message, file ? std::string(file) : "", line));
		}

		virtual const char* what() const noexcept;


		std::string message() const noexcept;



	public:




		

	public:


		struct Info {
			std::string message;	
			std::string file;		
			int         line;		

		public:
			Info(const std::string& iMessage, const std::string& iFile, int iLine) :
				message(iMessage), file(iFile), line(iLine) {}

			std::string what(std::size_t messageWidth) const;
		};


		const Info& getInfo() const 
		{ 
			assert(!mInfos.empty()); 
			return mInfos.front(); 
		}

	protected:
		std::list<Info> mInfos;

		mutable std::string mMessage; 
	};

}
