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

#pragma once

namespace claid
{
    template<typename T> struct is_integer_no_bool : public std::false_type {};
}

#define CLAID_IS_INTEGER_NO_BOOL(type) \
namespace claid\
{\
    template<>\
    struct is_integer_no_bool<type> : public std::true_type {};\
}

CLAID_IS_INTEGER_NO_BOOL(unsigned char)
CLAID_IS_INTEGER_NO_BOOL(signed char)

CLAID_IS_INTEGER_NO_BOOL(signed short)
CLAID_IS_INTEGER_NO_BOOL(unsigned short)

CLAID_IS_INTEGER_NO_BOOL(signed int)
CLAID_IS_INTEGER_NO_BOOL(unsigned int)

CLAID_IS_INTEGER_NO_BOOL(signed long)
CLAID_IS_INTEGER_NO_BOOL(unsigned long)

CLAID_IS_INTEGER_NO_BOOL(signed long long)
CLAID_IS_INTEGER_NO_BOOL(unsigned long long)


