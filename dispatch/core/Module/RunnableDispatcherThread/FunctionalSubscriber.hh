
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