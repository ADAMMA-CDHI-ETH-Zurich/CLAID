#include "dispatch/core/CLAID.hh"
#include "dispatch/core/Router/RoutingQueueMergerGeneric.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
#include "dispatch/core/Logger/Logger.hh"

#define CHECK_STATUS(status) if(!status.ok()) { std::cout << status << "\n"; exit(0); }

namespace claid
{
    // This function is currently only meant for testing.
    // The function currently does not return and either results in an endless loop (if CLAID was started successfull),
    // or in a termination of the app (if an error occured while starting).
    // Otherwise, a segfault might occur when exiting the function, since all variables are only locally.
    absl::Status start(const std::string& configurationPath, const std::string& currentHost)
    {
        // Parse configuration
        Configuration config;
        absl::Status status = config.parseFromJSONFile(configurationPath);

        CHECK_STATUS(status);
        HostDescriptionMap hostDescriptions;
        ModuleDescriptionMap moduleDescriptions;
        ChannelDescriptionMap channelDescriptions;

        status = config.getHostDescriptions(hostDescriptions);
        CHECK_STATUS(status);

        status = config.getModuleDescriptions(moduleDescriptions);
        CHECK_STATUS(status);

        status = config.getChannelDescriptions(channelDescriptions);
        CHECK_STATUS(status);

        // Populate module table
        ModuleTable moduleTable;
        status = populateModuleTable(moduleDescriptions, channelDescriptions, moduleTable);
        CHECK_STATUS(status);

        // Set up the routing queues
        SharedQueue<DataPackage>& localQueue = moduleTable.inputQueue();
        SharedQueue<DataPackage> clientQueue;
        SharedQueue<DataPackage> serverQueue;

        // Input queue for the MasterRouter
        claid::SharedQueue<DataPackage> masterQueue;

        // Start the queue merger
        // Merges the localQueue, clientQueue and serverQueue into the masterQueue.
        // (Spawns 3 threads that forward data from the 3 queues to the one masterQueue).
        RoutingQueueMergerGeneric merger(masterQueue, localQueue, clientQueue, serverQueue);
        status = merger.start();
        CHECK_STATUS(status);

        HostUserTable hostUserTable;
        std::shared_ptr<LocalRouter> localRouter = std::make_shared<LocalRouter>(moduleTable);
        std::shared_ptr<ClientRouter> clientRouter = std::make_shared<ClientRouter>();
        std::shared_ptr<ServerRouter> serverRouter = std::make_shared<ServerRouter>(hostUserTable);

        // Setup the router
        MasterRouter router(masterQueue, localRouter, clientRouter, serverRouter);
        status = router.buildRoutingTable(currentHost, hostDescriptions);
        CHECK_STATUS(status);

        status = router.start();
        CHECK_STATUS(status);

        Logger::printfln("CLAID started successfully");

        while(true);
    }

    absl::Status populateModuleTable(
        const ModuleDescriptionMap& moduleDescriptions,
        const ChannelDescriptionMap& channelDescriptions,
        ModuleTable& moduleTable)
    {
        Logger::printfln("Module size: %d", moduleDescriptions.size());
        for(const auto& entry : moduleDescriptions)
        {
            const ModuleDescription& moduleDescription = entry.second;

            moduleTable.setModule(moduleDescription.id,
                moduleDescription.moduleClass,
                moduleDescription.properties);
        }

        for(const auto& entry : channelDescriptions)
        {
            const ChannelDescription& channelDescription = entry.second;

            const std::string& channelName = channelDescription.channelName;

            for(const std::string& publisher : channelDescription.publisherModules)
            {
                for(const std::string& subscriber : channelDescription.subscriberModules)
                {
                    moduleTable.setChannel(channelName, publisher, subscriber);
                }
            }
        }

        return absl::OkStatus();
    }
}