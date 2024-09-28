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

#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include "dispatch/core/DataCollection/DataSaver/FileSaver.hh"
#include "dispatch/proto/sensor_data_types.pb.h"
#include "dispatch/core/CLAID.hh"

// Function to generate a random alphanumeric character
char getRandomChar() {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = sizeof(charset) - 1;
    return charset[rand() % max_index];
}

// Function to generate random content and write to file
void generateRandomFiles(int numFiles) {
    // Seed random number generator
    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < numFiles; ++i) {
        // Generate random file size between 1KB and 100KB
        int fileSize = (rand() % (100 * 1024 - 1 * 1024 + 1)) + 1 * 1024;

        // Create unique file name
        std::ostringstream fileName;
        fileName << "random_file_" << i<< ".txt";

        // Open file for writing
        std::ofstream outFile(fileName.str(), std::ios::binary);
        if (!outFile) {
            std::cerr << "Error creating file: " << fileName.str() << std::endl;
            continue;
        }

        // Generate random content and write to the file
        for (int j = 0; j < fileSize; ++j) {
            outFile.put(getRandomChar());
        }

        // Close the file
        outFile.close();

        std::cout << "File " << fileName.str() << " created with size " << fileSize << " bytes." << std::endl;
    }
}

using namespace claid;

// Creates two simple Modules (Sender, Receiver) and registers them with the C++ Runtime.
// Starts the C++ runtime and and waits for the Receiver to receive data from the Sender.
TEST(DataSyncModuleTestSuite, DataSyncModuleSetSyncScheduleTest)  
{

    // // unix domain socket for the client CLAID instance (not for remote communication, only for local dispatching!)
    // const char* socket_path_local_1 = "unix:///tmp/data_sync_module_test_client.sock";

    // // unix domain socket for the server CLAID instance (not for remote communication, only for local dispatching!)
    // const char* socket_path_local_2 = "unix:///tmp/data_sync_module_test_server.sock";
    // const char* config_file = "dispatch/test/data_sync_module_test.json";
    // const char* client_host_id = "test_client";
    // const char* server_host_id = "test_server";
    // const char* user_id = "user42";
    // const char* device_id = "something_else";

    // bool result;

    // Logger::logInfo("===== STARTING CLIENT MIDDLEWARE ====");

    // CLAID clientMiddleware;
    // result = clientMiddleware.start(socket_path_local_1, config_file, client_host_id, user_id, device_id);
    // ASSERT_TRUE(result) << "Failed to start client middleware";
   
    // // We deliberately start the clientMiddleware first. This will cause the connection attempt
    // // of the RemoteDispatcherClient to fail, because the server is not started yet.
    // // The RemoteDispatcherClient should then try at a later time (every 2 seconds);
    // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    // Logger::logInfo("===== STARTING SERVER MIDDLEWARE ====");
    
    // CLAID serverMiddleware;
    // result = serverMiddleware.start(socket_path_local_2, config_file, server_host_id, user_id, device_id);
    // ASSERT_TRUE(result) << "Failed to start server middleware";
   

    // std::this_thread::sleep_for(std::chrono::milliseconds(4100));
    // ASSERT_TRUE(clientMiddleware.isConnectedToRemoteServer()) << clientMiddleware.getRemoteClientStatus();

    // // while(dataSentCtr != dataReceivedCtr || dataSentCtr != REQUIRED_DATA_TRANSMISSIONS)
    // // {
    // //     Logger::logInfo(
    // //         """Waiting for data transmissions and receivals, current is:\n"""
    // //         """Transmissions: %d/%d\n"""
    // //         """Receivals: %d/%d\n""",
    // //         dataSentCtr, REQUIRED_DATA_TRANSMISSIONS,
    // //         dataReceivedCtr, REQUIRED_DATA_TRANSMISSIONS
    // //     );
    // //     std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // // }

    // Logger::logInfo("===== STOPPING SERVER MIDDLEWARE ====");

    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // serverMiddleware.shutdown();
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // Logger::logInfo("===== STOPPING CLIENT MIDDLEWARE ====");

    // clientMiddleware.shutdown();

    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // // ASSERT_EQ(dataSentCtr, REQUIRED_DATA_TRANSMISSIONS) << "Invalid number of data transmissions! Expected " << REQUIRED_DATA_TRANSMISSIONS << " but got " << dataSentCtr;
    // // ASSERT_EQ(dataReceivedCtr, REQUIRED_DATA_TRANSMISSIONS) << "Invalid number of data receivals! Expected " << REQUIRED_DATA_TRANSMISSIONS << " but got " << dataReceivedCtr;

}
