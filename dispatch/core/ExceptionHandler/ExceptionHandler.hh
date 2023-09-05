#pragma once
#include <functional>
#include <string>

namespace claid
{

	class ExceptionHandler
	{
		private:
			static bool exceptionHandlerRegistered;
			static std::function<void (std::string, std::string file, int line)> exceptionFunction;

		public:

			static bool isExeceptionHandlerRegistered();
			static void registerExceptionHandler(std::function<void (std::string, std::string, int)> exceptionFunction);
			static void invokeExceptionHandler(std::string exception, std::string file, int line);
	};
}