#include "FuturesTable.hh"
#include "AbstractFuture.hh"


namespace claid {

void FuturesTable::addFuture(std::shared_ptr<AbstractFuture> future)
{
    std::unique_lock<std::mutex>(this->lock);
    futures.insert(std::make_pair(future->getUniqueIdentifier(), future));
}

bool FuturesTable::removeFuture(FutureUniqueIdentifier futureIdentifier)
{
    std::unique_lock<std::mutex>(this->lock);

    auto it = futures.find(futureIdentifier);
    if(it != futures.end())
    {
        std::shared_ptr<AbstractFuture> other = it->second;
     
        futures.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<AbstractFuture> FuturesTable::lookupFuture(FutureUniqueIdentifier uniqueIdentifier)
{
    std::shared_ptr<AbstractFuture> future = nullptr;

    std::unique_lock<std::mutex>(this->lock);

    auto it = futures.find(uniqueIdentifier);
    if(it != futures.end())
    {
        future = it->second;
    }
    return future;
}

}