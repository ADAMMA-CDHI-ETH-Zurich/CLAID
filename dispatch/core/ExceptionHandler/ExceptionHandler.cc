/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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

