#include "AbstractFuture.hh"
#include "FuturesTable.hh"
#include "FutureUniqueIdentifier.hh"

namespace claid {


AbstractFuture::AbstractFuture(FuturesTable& futuresTableInHandler, 
        FutureUniqueIdentifier uniqueIdentifier)  : futuresTableInHandler(futuresTableInHandler), uniqueIdentifier(uniqueIdentifier)
{
}


std::shared_ptr<DataPackage> AbstractFuture::awaitResponse()
{
    std::unique_lock<std::mutex> lock(this->mutex);
  
    while (!finished) 
    {                
        conditionVariable.wait(lock, [&]{return this->finished;});  
    }


    // This is thread safe, as FuturesList is thread safe
    this->futuresTableInHandler.removeFuture(this->uniqueIdentifier);

    if(this->successful)
    {
        return this->responsePackage;
    }
    // Otherwise, null will be returned
    
    
    return nullptr;
}


void AbstractFuture::thenUntyped(ThenCallback callback)
{
    this->callback = callback;
    this->callbackSet = true;
}

void AbstractFuture::setResponse(std::shared_ptr<DataPackage> responsePackage) 
{
    std::unique_lock<std::mutex>(this->mutex);

    this->responsePackage = responsePackage;
    this->successful = true;

    if(this->callback)
    {
        this->callback(responsePackage);
    }

    this->finished = true;
    this->conditionVariable.notify_all();
}

void AbstractFuture::setFailed()
{
    std::unique_lock<std::mutex>(this->mutex);
    this->responsePackage = nullptr;
    this->successful = false;
    
    if(this->callbackSet)
    {
        this->callback(nullptr);
    }

    this->finished = true;
    this->conditionVariable.notify_all();
}

FutureUniqueIdentifier AbstractFuture::getUniqueIdentifier() const
{
    return this->uniqueIdentifier;
}

bool AbstractFuture::wasExecutedSuccessfully() const
{
    return this->successful && this->finished;
}

}