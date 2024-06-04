// #pragma once

// #include "dispatch/proto/claidservice.grpc.pb.h"
// #include "AbstractMutatorHelper.hh"
// #include <tuple>

// using namespace claid;

// namespace claid {

// template<typename... PayloadTypes>
// struct PayloadHelper
// {
//     private:
//         std::vector<std::shared_ptr<AbstractMutatorHelper>> mutatorHelpers;

//     public:

//         PayloadHelper(std::vector<std::shared_ptr<AbstractMutatorHelper>> mutatorHelpers) : mutatorHelpers(mutatorHelpers)
//         {

//         }

//         template<int C, typename U, typename... Us>
//         void setParameterPayloads(RemoteFunctionRequest& request, std::tuple<PayloadTypes*...> data)
//         {
//             DataPackage stubPackage;

//             std::shared_ptr<AbstractMutatorHelper> helper = this->mutatorHelpers[i];
//             helper->setPackagePayload(stubPackage, *std::get<C>(data));

//             (*request.add_parameter_payloads()) = stubPackage.payload();

//             setParameterPayloads<C + 1, Us...>(request, data);
//         }

//         template<int C>
//         void setParameterPayloads(RemoteFunctionRequest& request, U& parameter)
//         {
//         }
        
// };

// }