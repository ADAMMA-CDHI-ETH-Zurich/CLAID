#include "dispatch/core/ExceptionHandler/ExceptionHandler.hh"


bool claid::ExceptionHandler::exceptionHandlerRegistered = false;
std::function<void (std::string, std::string, int)> claid::ExceptionHandler::exceptionFunction;


bool claid::ExceptionHandler::isExeceptionHandlerRegistered()
{
	return claid::ExceptionHandler::exceptionHandlerRegistered;
}

void claid::ExceptionHandler::registerExceptionHandler(std::function<void (std::string, std::string, int)> exceptionFunction)
{
	claid::ExceptionHandler::exceptionFunction = exceptionFunction;
	claid::ExceptionHandler::exceptionHandlerRegistered = true;
}

void claid::ExceptionHandler::invokeExceptionHandler(std::string exception, std::string file, int line)
{
	if(claid::ExceptionHandler::exceptionHandlerRegistered)
	{
		claid::ExceptionHandler::exceptionFunction(exception, file, line);
	}
}

