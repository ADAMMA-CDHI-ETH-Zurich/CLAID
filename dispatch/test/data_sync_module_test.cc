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
#include "dispatch/core/Utilities/FileUtils.hh"

using namespace claid;

const std::string FOLDER_PATH = "source_files";
const std::string OUTPUT_FOLDER_PATH = "synchronized_files";
const std::string USER_NAME = "user42";
CLAID clientMiddleware;
CLAID serverMiddleware;

struct FileInfo
{
    std::string fileName;
    uint64_t fileSize;

    FileInfo() {}
    FileInfo(const std::string& fileName, uint64_t fileSize) : fileName(fileName), fileSize(fileSize)
    {

    }
};

// Function to generate a random alphanumeric character
char getRandomChar() {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = sizeof(charset) - 1;
    return charset[rand() % max_index];
}

// Function to generate random content and write to file
void  generateRandomFiles(int numFiles, std::vector<FileInfo>& files) {
    // Seed random number generator
    srand(static_cast<unsigned int>(time(0)));

    files.clear();

    for (int i = 0; i < numFiles; ++i) {
        // Generate random file size between 1KB and 100KB
        const int minKiloBytes = 10;
        const int maxKiloBytes = 100;
        int fileSize = (rand() % (maxKiloBytes * 1024 - minKiloBytes * 1024 + 1)) + minKiloBytes * 1024;

        // Create unique file name
        std::ostringstream fileName;
        fileName << "random_file_" << i << ".txt";
        files.push_back(FileInfo(fileName.str(), fileSize));
        fileName.str("");
        fileName.clear();
        fileName << FOLDER_PATH << "/random_file_" << i<< ".txt";

        // Open file for writing
        std::ofstream outFile(fileName.str(), std::ios::binary);
        
        ASSERT_TRUE(outFile.is_open()) << "Failed to open file " << fileName.str();



        // Generate random content and write to the file
        for (int j = 0; j < fileSize; ++j) {
            outFile.put(getRandomChar());
        }

        // Close the file
        outFile.close();

        std::cout << "File " << fileName.str() << " created with size " << fileSize << " bytes." << std::endl;
    }
}




void clearOrCreateDataFolder()
{
    if(FileUtils::dirExists(FOLDER_PATH))
    {
        FileUtils::removeDirectoryRecursively(FOLDER_PATH);
    }
    std::string cwd;
    ASSERT_TRUE(FileUtils::getCurrentWorkingDirectory(cwd));

    std::string absolutePath = absl::StrCat(cwd, "/", FOLDER_PATH);
    ASSERT_TRUE(FileUtils::createDirectory(FOLDER_PATH)) << "Failed to create storage folder " << absolutePath;
    Logger::logInfo("Data folder created under %s", absolutePath.c_str());
}


void startCLAIDInstances()
{
    // unix domain socket for the client CLAID instance (not for remote communication, only for local dispatching!)
    const char* socket_path_local_1 = "unix:///tmp/data_sync_module_test_client.sock";

    // unix domain socket for the server CLAID instance (not for remote communication, only for local dispatching!)
    const char* socket_path_local_2 = "unix:///tmp/data_sync_module_test_server.sock";
    const char* config_file = "dispatch/test/data_sync_module_test.json";
    const char* client_host_id = "test_client";
    const char* server_host_id = "test_server";
    const char* user_id = USER_NAME.c_str();
    const char* device_id = "something_else";

    bool result;

    Logger::logInfo("===== STARTING CLIENT MIDDLEWARE ====");

    std::string workingDirectory;
    ASSERT_TRUE(FileUtils::getCurrentWorkingDirectory(workingDirectory)) << "Failed to get working directory";
    result = clientMiddleware.start(socket_path_local_1, config_file, client_host_id, user_id, device_id, workingDirectory);
    ASSERT_TRUE(result) << "Failed to start client middleware";
   
    // We deliberately start the clientMiddleware first. This will cause the connection attempt
    // of the RemoteDispatcherClient to fail, because the server is not started yet.
    // The RemoteDispatcherClient should then try at a later time (every 2 seconds);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    Logger::logInfo("===== STARTING SERVER MIDDLEWARE ====");
    
    result = serverMiddleware.start(socket_path_local_2, config_file, server_host_id, user_id, device_id, workingDirectory);
    ASSERT_TRUE(result) << "Failed to start server middleware";
   

    std::this_thread::sleep_for(std::chrono::milliseconds(4100));
    ASSERT_TRUE(clientMiddleware.isConnectedToRemoteServer()) << clientMiddleware.getRemoteClientStatus();
}

void stopCLAIDInstances()
{
    Logger::logInfo("===== STOPPING SERVER MIDDLEWARE ====");
    serverMiddleware.shutdown();

    Logger::logInfo("===== STOPPING CLIENT MIDDLEWARE ====");

    clientMiddleware.shutdown();

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    Logger::logInfo("===== STOPPED ALL CLAID INSTANCES ====");
}

void checkSynchronizedFiles(const std::vector<FileInfo>& files)
{
    std::vector<std::string> fileNames;
    
    const std::string synchronizedPath = Path::join(OUTPUT_FOLDER_PATH, USER_NAME);

    ASSERT_TRUE(FileUtils::getAllFilesInDirectory(synchronizedPath, fileNames)) << "Failed to get files in output folder " << synchronizedPath;
    ASSERT_EQ(fileNames.size(), files.size()) << "Invalid number of files! Expected " << files.size() << " but got " << fileNames.size();

    Logger::logInfo("The following files were found in output folder %s:", synchronizedPath.c_str());
    for(int i = 0; i < fileNames.size(); i++)
    {
        // Remove path prefix
        std::string& filePath = fileNames[i];
        std::string fileName = Path(filePath).getFileNameFromPath();
        Logger::logInfo("\t%s -> %s", filePath.c_str(), fileName.c_str());
        filePath = fileName;
    }

    for(const FileInfo& file : files)
    {
        const std::string& fileName = file.fileName;
        ASSERT_NE(std::find(fileNames.begin(), fileNames.end(), fileName), fileNames.end()) << "File \"" << fileName << "\" not found in folder " << synchronizedPath << "\n";

        uint64_t fileSize;
        std::string filePath = Path::join(OUTPUT_FOLDER_PATH, USER_NAME, fileName);
        ASSERT_TRUE(FileUtils::getFileSize(filePath, fileSize)) << "Failed to get size of file \"" << filePath << "\".";
        ASSERT_EQ(fileSize, file.fileSize) << "Invalid file size for file \"" << filePath << "\". Expected " << fileSize << " but got " << file.fileSize;
    }

    // Check that original files have been deleted, as deleteAfterSync is true.
    std::vector<std::string> leftoverFiles;
    ASSERT_TRUE(FileUtils::getAllFilesInDirectory(FOLDER_PATH, leftoverFiles)) << "Failed to get files in folder " << FOLDER_PATH;

    ASSERT_EQ(leftoverFiles.size(), 0) << "Leftover files found in directory" << FOLDER_PATH << "directory. Directory should be empty!";
}

void testCase1()
{
    Logger::logInfo("\n\n========== TEST CASE 1 ==========\n");
    // In test case 1, we generate 10 random files and synchronize them to an output folder.
    // The 10 files should be deleted from the source folder during synchronization.
    // Expectation:
    //  - output folder should contain the 10 files, with correct sizes, after max 10 seconds.
    //  - source folder should be empty after max 10 seconds.
    std::vector<FileInfo> files;
    generateRandomFiles(10, files);
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    checkSynchronizedFiles(files);
}

void testCase2()
{
    Logger::logInfo("\n\n========== TEST CASE 2 ==========\n");
    // In test case 2, we now have 21 files. The first 10 are named the same as in test case 1. Those 10 files
    // are still available in the output folder. Now, since we generate 21 new files, the first 10 will have different sizes.
    // Thus, they should again be synchronized, and deleted in the process.
    // Expectation:
    //  - output folder should contain the 21 files, with correct sizes, after max 10 seconds.
    //  - source folder should be empty after max 10 seconds.
    std::vector<FileInfo> files;
    generateRandomFiles(21, files);
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    checkSynchronizedFiles(files);
}

void testCase3(std::vector<FileInfo>& returnFiles)
{
    Logger::logInfo("\n\n========== TEST CASE 3 ==========\n");
    // In test case 3, we set the syncing schedule of the data sync module to an empty schedule, which should effectively 
    // disable the synchronization. Before starting, we clear the output folder. We create 15 in the source folder.
    // Expectation:
    // - output folder should be empty
    // - source folder should contain 15 files
    clearOrCreateDataFolder();
    RemoteFunctionHandler* remoteFunctionHandler = clientMiddleware.getRemoteFunctionHandler();
    ASSERT_NE(remoteFunctionHandler, nullptr) << "RemoteFunctionHandler is null.";

    RemoteFunction<void> setSyncingSchedule = remoteFunctionHandler->mapModuleFunction<void, Schedule>("DataSyncer", "set_syncing_schedule");

    // Set empty schedule
    Schedule schedule;
    auto future = setSyncingSchedule.execute(schedule);
    future->await();
    ASSERT_TRUE(future->wasExecutedSuccessfully()) << "Remote function \"set_syncing_schedule\" was not executed successfully on DataSyncer Module";
    generateRandomFiles(15, returnFiles);
    std::this_thread::sleep_for(std::chrono::milliseconds(12000));

    std::vector<std::string> leftoverFiles;
    ASSERT_TRUE(FileUtils::getAllFilesInDirectory(FOLDER_PATH, leftoverFiles)) << "Failed to get files in folder " << FOLDER_PATH;
    ASSERT_EQ(leftoverFiles.size(), 15) << "Files have been synchronized even though they should not have been! Expected 0 files but found " << leftoverFiles.size();
}

void testCase4(const std::vector<FileInfo>& files)
{
    Logger::logInfo("\n\n========== TEST CASE 4 ==========\n");
    // In test case 4, we set the syncing schedule of the data sync module to the previously schedule, effectively continuing 
    // the synchronization. We then simply wait for the leftover files from testCase3 to be synchronized.
    // Before we set the schedule, we clear the output folder.
    // Expectation:
    // - output folder should be empty
    // - source folder should contain 15 files

    const std::string synchronizedPath = Path::join(OUTPUT_FOLDER_PATH, USER_NAME);

    ASSERT_TRUE(FileUtils::removeDirectoryRecursively(synchronizedPath)) << "Failed to clear output directory " << synchronizedPath;
    ASSERT_TRUE(FileUtils::createDirectoriesRecursively(synchronizedPath)) << "Failed to create output directory " << synchronizedPath;

    RemoteFunctionHandler* remoteFunctionHandler = clientMiddleware.getRemoteFunctionHandler();
    ASSERT_NE(remoteFunctionHandler, nullptr) << "RemoteFunctionHandler is null.";

    RemoteFunction<void> setSyncingSchedule = remoteFunctionHandler->mapModuleFunction<void, Schedule>("DataSyncer", "set_syncing_schedule");

    // Set empty schedule
    Schedule schedule;
    SchedulePeriodic* periodic_entry = schedule.add_periodic();
    periodic_entry->set_period_seconds(5);

    auto future = setSyncingSchedule.execute(schedule);
    future->await();
    ASSERT_TRUE(future->wasExecutedSuccessfully()) << "Remote function \"set_syncing_schedule\" was not executed successfully on DataSyncer Module";

    std::this_thread::sleep_for(std::chrono::milliseconds(12000));
    checkSynchronizedFiles(files);
}

// Creates two simple Modules (Sender, Receiver) and registers them with the C++ Runtime.
// Starts the C++ runtime and and waits for the Receiver to receive data from the Sender.
TEST(DataSyncModuleTestSuite, DataSyncModuleSetSyncScheduleTest)  
{
    clearOrCreateDataFolder();

    startCLAIDInstances();

    testCase1();
    testCase2();
    
    std::vector<FileInfo> files;
    testCase3(files); // Fills files
    testCase4(files); // Uses files

    stopCLAIDInstances();
}
