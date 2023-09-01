#ifndef CLAID_CONFIG_HH_
#define CLAID_CONFIG_HH_

// #include <algorithm>
// #include <chrono>
// #include <cmath>
// #include <iostream>
#include <memory>
#include <string>

#include "dispatch/proto/claidconfig.pb.h"

using namespace std;

using claidservice::ClaidConfig;

namespace claid {

// TODO: Maybe add a way to return an error. 
shared_ptr<ClaidConfig> parseConfigFile(const std::string& configFile);

}  // namespace claid 

#endif  // CLAID_CONFIG_H_
