#ifndef CLAID_CONFIG_HH_
#define CLAID_CONFIG_HH_

// #include <algorithm>
// #include <chrono>
// #include <cmath>
// #include <iostream>
#include <memory>
#include <string>
#include "absl/status/status.h"

#include "dispatch/proto/claidconfig.pb.h"

using namespace std;


namespace claid {

bool loadConfigFileToString(const std::string& filePath, std::string& buffer);

absl::Status parseConfigFile(const std::string& configFilePath, shared_ptr<claidservice::CLAIDConfig>& configPtr);
absl::Status configToJson(std::shared_ptr<claidservice::CLAIDConfig> config, std::string& buffer);

}  // namespace claid 

#endif  // CLAID_CONFIG_H_
