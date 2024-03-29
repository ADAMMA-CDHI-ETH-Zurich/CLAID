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

#include "dispatch/core/CLAID.hh"
// Include the CLAID ArgumentParser
#include "dispatch/core/Utilities/ArgumentParser.hh"
// Change main to inlcude argc and argv parameters.
int main(int argc, char** argv) 
{
    ArgumentParser parser(argc, argv);

    std::string configPath = "";
    // Register argument to the parser.
    // This automatically parses argv.
    // A default value can be specified by the last parameter of the function.
    parser.add_argument<std::string>("path", configPath, "");

    // If "path" was not specified in argv, configPath is empty.
    // In that case, we print an error message.
    if(configPath == "")
    {
        printf("Error, no path to JSON configuration specified.\n"
        "Please use ./claidcmd path path/to/json/file.\n");
        exit(0);
    }

    const char* socket_path = "/tmp/test_grpc.sock";
    const char* config_file = configPath.c_str();
    const char* host_id = "test_server";
    const char* user_id = "user42";
    const char* device_id = "something_else";

    claid::CLAID claid;
    bool result = claid.start(socket_path, config_file, host_id, user_id, device_id);
    while(true);
    return 0;
}
