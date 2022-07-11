#pragma once
#include <assert.h>

#include <string.h>

#include <exception>
#include <sstream>
#include <iostream>

#include <list>
#include "Logger/Logger.hpp"


#define PORTAIBLE_THROW(ex, msg)\
{\
	std::ostringstream ex_str;\
	ex_str << msg;\
	Logger::printfln("%s", ex_str.str().c_str());\
	throw ex(ex_str.str(), __FILE__, __LINE__); \
}

namespace portaible
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
			std::string message;	/// The exception message
			std::string file;		/// The file the exception occurred
			int         line;		/// The line the exception occurred

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

		mutable std::string mMessage; ///< as cache for what()
	};

}