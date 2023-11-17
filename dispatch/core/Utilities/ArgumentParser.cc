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
#include "dispatch/core/Utilities/ArgumentParser.hh"

#include <stdio.h>
#include <string.h>

#include <algorithm>

ArgumentParser::ArgumentParser(int argc, char** argv) : argc(argc), argv(argv)
{

}

void ArgumentParser::printHelpIfInvalidArgumentFound()
{
    bool unknownArgumentFound = false;
    // Ignore first argument, it is the name of the application
    // All arguments with even numbers are the values of the parameters itself.
    // So ignore them to! We only check odd arguments.
    for(int i = 1; i < this->argc; i += 2)
    {
        
        std::string argumentStr = std::string(argv[i]);

        if(std::find(this->arguments.begin(), this->arguments.end(), argumentStr) == this->arguments.end())
        {
            // Unknown argument found!
            unknownArgumentFound = true;
            break;
        }
    }

    if(unknownArgumentFound)
    {
        printf("Usage: \n");
        for(size_t i = 0; i < this->arguments.size(); i++)
        {
            printf("%s: \"%s\"\n", this->arguments[i].c_str(), this->descriptions[i].c_str());
        }
    }
}

int ArgumentParser::findArgumentIndex(const char* argument)
{
    for(int i = 0; i < this->argc; i++)
    {
        if(strcmp(argument, this->argv[i]) == 0)
        {
            return i;
        }
    }

    return -1;
}

template<> void ArgumentParser::add_argument<int>(std::string argument, int& var, int defaultValue, std::string description)
{
    int index = this->findArgumentIndex(argument.c_str());
    
    if(index != -1)
    {
        var = std::stoi(this->argv[index + 1]);
    }
    else
    {
        var = defaultValue;
    }
    this->arguments.push_back(argument);
    this->descriptions.push_back(description);
}

template<> void ArgumentParser::add_argument<float>(std::string argument, float& var, float defaultValue, std::string description)
{
    int index = this->findArgumentIndex(argument.c_str());
    
    if(index != -1)
    {
        var = std::stof(this->argv[index + 1]);
    }
    else
    {
        var = defaultValue;
    }
    this->arguments.push_back(argument);
    this->descriptions.push_back(description);
}

template<> void ArgumentParser::add_argument<double>(std::string argument, double& var, double defaultValue, std::string description)
{
    int index = this->findArgumentIndex(argument.c_str());
    
    if(index != -1)
    {
        var = std::stod(this->argv[index + 1]);
    }
    else
    {
        var = defaultValue;
    }
    this->arguments.push_back(argument);
    this->descriptions.push_back(description);
}

template<> void ArgumentParser::add_argument<long>(std::string argument, long& var, long defaultValue, std::string description)
{
    int index = this->findArgumentIndex(argument.c_str());
    
    if(index != -1)
    {
        var = std::stol(this->argv[index + 1]);
    }
    else
    {
        var = defaultValue;
    }
    this->arguments.push_back(argument);
    this->descriptions.push_back(description);
}

template<> void ArgumentParser::add_argument<short>(std::string argument, short& var, short defaultValue, std::string description)
{
    int index = this->findArgumentIndex(argument.c_str());
    
    if(index != -1)
    {
        var = static_cast<short>(std::stoi(this->argv[index + 1]));
    }
    else
    {
        var = defaultValue;
    }
    this->arguments.push_back(argument);
    this->descriptions.push_back(description);
}

template<> void ArgumentParser::add_argument<bool>(std::string argument, bool& var, bool defaultValue, std::string description)
{
    int index = this->findArgumentIndex(argument.c_str());
    
    if(index != -1)
    {
        var = static_cast<bool>(std::stoi(this->argv[index + 1]));
    }
    else
    {
        var = defaultValue;
    }
    this->arguments.push_back(argument);
    this->descriptions.push_back(description);
}

template<> void ArgumentParser::add_argument<std::string>(std::string argument, std::string& var, std::string defaultValue, std::string description)
{
    int index = this->findArgumentIndex(argument.c_str());
    
    if(index != -1)
    {
        var =  std::string(argv[index + 1]);
    }
    else
    {
        var = defaultValue;
    }
    this->arguments.push_back(argument);
    this->descriptions.push_back(description);
}