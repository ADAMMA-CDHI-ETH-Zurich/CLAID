#include "dispatch/core/Router/Router.hh"

namespace claid
{

    absl::Status Router::initialize()
    {
        return absl::OkStatus();
    }

    void Router::runRouting()
    {
        while(this->running)
        {
            std::shared_ptr<DataPackage> package;
            package = this->inputQueue.pop_front();
            this->routePackage(package);
        }
    }

    absl::Status Router::start() 
    {
        absl::Status status = initialize();

        if(!status.ok())
        {
            return status;
        }

        this->running = true;

        routingThread = make_unique<thread>([this]() { runRouting(); });
        return absl::OkStatus();
    }

    void Router::enqueuePackageForRouting(std::shared_ptr<DataPackage> dataPackage)
    {
        this->inputQueue.push_back(dataPackage);
    }

  
}