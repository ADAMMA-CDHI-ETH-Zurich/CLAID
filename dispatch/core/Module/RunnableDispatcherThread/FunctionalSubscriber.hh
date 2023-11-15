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
// REQUIRES C++17, thus deactivated for now.

// #include "RunnableDispatcherThread/RunnableDispatcherThread.hh"
// #include <functional>

// #include "SubscriberBase.hh"
// #include "ChannelData.hh"
// #include "mutex"
// #include <iostream>



// namespace claid
// {
//     // forward declaration
//     template<typename T>
//     class TypedChannel;

//     template<typename Return, typename... Ts>
//     class FunctionalSubscriber : public FunctionRunnableWithParams<Return, Ts...>
//     {   
     
      


//         public:

//             FunctionalSubscriber(Sub* runnableDispatcherThread,
// 	                  std::function<Return (Ts...)> function) : SubscriberBase(runnableDispatcherThread), FunctionRunnableWithParams(function)
//             {
//                 this->lastTimeStamp = Time::now();
//             }

     
            

//             Runnable* asRunnable()
//             {
//                 return static_cast<Runnable*>(this);
//             }

//         private:
//             SubscriberBase

             
//     };  
// }