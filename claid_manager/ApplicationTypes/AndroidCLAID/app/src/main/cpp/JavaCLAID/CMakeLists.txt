set(CMAKE_CXX_STANDARD 17)


        include_directories(/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/jbind11/CMakeUtils/../jni_stub)
        
add_executable(jbind11_wrapper_deployer_JavaCLAID /Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/jbind11/src/deploy.cpp
/Users/planger/AndroidStudioProjects/AndroidCLAID/app/src/main/cpp/JavaCLAID.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Buffer/Buffer.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Buffer/BufferInfo.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/CSV/CSVSerializer.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Channel/Channel.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Channel/ChannelManager.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Channel/TaggedDataBase.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/ClassFactory/ClassFactory.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Exception/Exception.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/File/File.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Logger/Logger.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Module/Module.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Module/ModuleFactory/ModuleFactory.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Namespace/NamespaceXMLLoader.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Network/NetworkClientModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Network/NetworkServerModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Network/NetworkStateChangeRequest.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Network/Socket/Client/SocketClient.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Network/Socket/Server/ClientAcceptModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Network/Socket/Server/SocketServer.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Network/SocketConnectionModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Network/SocketReaderModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/ConnectionLink.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/ConnectionModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageData/MessageData.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageData/MessageDataBinary.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageData/MessageDataEmpty.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageData/MessageDataString.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageHeader/MessageHeader.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageHeader/MessageHeaderChannelData.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageHeader/MessageHeaderChannelUpdate.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageHeader/MessageHeaderInitialListOfPublishedAndSubscribedChannels.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageHeader/MessageHeaderKeepAlive.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageHeader/MessageHeaderKeepAliveResponse.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/Message/MessageHeader/TestMessage.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/RemoteConnectedEntity.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/RemoteModule/LocalObserver.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/RemoteModule/RemoteModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RemoteConnection/RemoteModule/RemoteObserver.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RunTime/RunTime.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RunnableDispatcherThread/DispatcherThreadTimer.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RunnableDispatcherThread/RunnableDispatcherThread.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/RunnableDispatcherThread/TimerBase.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Serialization/string.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Utilities/ArgumentParser.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/Utilities/FileUtils.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XML/XMLDeserializer.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XML/XMLDocument.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XML/XMLSerializer.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XMLLoader/XMLModuleLoader.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XMLModules/XMLConfigFlashDescription.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XMLModules/XMLConfigFlashedModulesDescription.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XMLModules/XMLConfigFlasherModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XMLModules/XMLFlasherUniqueIdentifier.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XMLModules/XMLReceiverModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/src/XMLModules/XMLSenderModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaCLAID.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaCLAIDDataTypes.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaWrapper/AudioDataWrapper.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaWrapper/ChannelWrapper.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaWrapper/JavaModule.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaWrapper/JavaWrapperMaster.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaWrapper/ModuleWrapper.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaWrapper/Wrapper.cpp
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/src/JavaXMLLoader/JavaXMLLoader.cpp
)

target_link_directories(jbind11_wrapper_deployer_JavaCLAID PRIVATE
/Users/planger/AndroidStudioProjects/AndroidCLAID/app/src/main/cpp/TARGET_LINK_DIRECTORIES-NOTFOUND
)

target_include_directories(jbind11_wrapper_deployer_JavaCLAID PRIVATE
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/ModuleAPI/include
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/include
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/jbind11/include
/Users/planger/Development/PortAIbleStandaloneTest/CLAID/packages/JavaCLAID/jbind11/include/jbind11
)

add_definitions(-D__JAVA_WRAPPERS__)
add_definitions(-D__JBIND11_CROSSPLATFORM_ANDROID__)
get_property(DEPLOYER_EXECUTABLE_LOCATION TARGET jbind11_wrapper_deployer_JavaCLAID PROPERTY LOCATION)
   
        add_custom_command(TARGET jbind11_wrapper_deployer_JavaCLAID POST_BUILD
            COMMAND ${DEPLOYER_EXECUTABLE_LOCATION} /Users/planger/AndroidStudioProjects/AndroidCLAID/app/src/main/cpp/JavaCLAID 1
            WORKING_DIRECTORY /Users/planger/AndroidStudioProjects/AndroidCLAID/app/src/main/cpp
            COMMENT "Deploying java files."
        )

        add_custom_command(TARGET jbind11_wrapper_deployer_JavaCLAID POST_BUILD
            COMMAND /Users/planger/Library/Android/sdk/cmake/3.22.1/bin/cmake -E remove cmake_install.cmake
        )

        add_custom_command(TARGET jbind11_wrapper_deployer_JavaCLAID POST_BUILD
            COMMAND /opt/homebrew/Cellar/cmake/3.23.2/bin/cmake -E remove cmake_install.cmake
        )

        add_custom_command(TARGET jbind11_wrapper_deployer_JavaCLAID POST_BUILD
            COMMAND /opt/homebrew/Cellar/cmake/3.23.2/bin/cmake -E remove CMakeCache.txt
        )

        add_custom_command(TARGET jbind11_wrapper_deployer_JavaCLAID POST_BUILD
            COMMAND /opt/homebrew/Cellar/cmake/3.23.2/bin/cmake -E remove CMakeLists.txt
        )

        add_custom_command(TARGET jbind11_wrapper_deployer_JavaCLAID POST_BUILD
            COMMAND /opt/homebrew/Cellar/cmake/3.23.2/bin/cmake -E remove jbind11_wrapper_deployer_JavaCLAID
        )
        
        add_custom_command(TARGET jbind11_wrapper_deployer_JavaCLAID POST_BUILD
            COMMAND /opt/homebrew/Cellar/cmake/3.23.2/bin/cmake -E remove -f CMakeFiles
        )

        add_custom_command(TARGET jbind11_wrapper_deployer_JavaCLAID POST_BUILD
            COMMAND /opt/homebrew/Cellar/cmake/3.23.2/bin/cmake -E remove -f MakeFile
        )

        