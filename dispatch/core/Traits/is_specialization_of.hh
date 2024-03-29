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
#include <type_traits>
// Can be used to check whether a given type is a template 
// specialization of a (generic / templated) class.


// E.g.:
// is_specialization_of<std::vector<int>, std::vector> // true, because is std::vector<int> is a specialization of std::vector.
// is_specialization_of<std::string, std::vector>      // false, because string has nothing to do with std::vector.
template< class T, template<class...> class Primary >
struct is_specialization_of : std::false_type
{

};

template< template<class...> class Primary, class... Args >
struct is_specialization_of< Primary<Args...>, Primary> : std::true_type
{
    
};