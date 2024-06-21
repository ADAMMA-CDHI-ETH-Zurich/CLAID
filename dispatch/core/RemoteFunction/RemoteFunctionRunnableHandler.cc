#include "RemoteFunctionRunnableHandler.hh"

namespace claid {

RemoteFunctionRunnableHandler::RemoteFunctionRunnableHandler(const std::string& entityName,
        SharedQueue<DataPackage>& toMiddlewareQueue) : entityName(entityName), toMiddlewareQueue(toMiddlewareQueue)
{

}

bool RemoteFunctionRunnableHandler::addRunnable(std::string functionName, std::shared_ptr<AbstractRemoteFunctionRunnable> runnable)
{
    auto it = this->registeredRunnables.find(functionName);

    if(it != this->registeredRunnables.end())
    {
        return false;
    }

    this->registeredRunnables.insert(std::make_pair(functionName, runnable));
    return true;
}

bool RemoteFunctionRunnableHandler::executeRemoteFunctionRunnable(std::shared_ptr<DataPackage> rpcRequest)
{

    RemoteFunctionRequest request;
    if(!rpcRequest->control_val().has_remote_function_request())
    {
        Logger::logError("Failed to execute RPC request data package. Could not find definition of RemoteFunctionRequest.");
        return false;
    }

    request = rpcRequest->control_val().remote_function_request();

    RemoteFunctionIdentifier remoteFunctionIdentifier = request.remote_function_identifier();

    std::string functionName = remoteFunctionIdentifier.function_name();
    
    auto it = this->registeredRunnables.find(functionName);

    if(it == this->registeredRunnables.end())
    {
        Logger::logError("Failed to execute RPC request. Entity \"%s\" does not have a registered remote function called \"%s\".", 
            this->entityName.c_str(), functionName.c_str());
        return false;
    }

    std::shared_ptr<AbstractRemoteFunctionRunnable> runnable = it->second;
    std::shared_ptr<DataPackage> response = runnable->executeRemoteFunctionRequest(rpcRequest);

    if(response != nullptr)
    {
        this->toMiddlewareQueue.push_back(response);
    }

    return true;
}

}