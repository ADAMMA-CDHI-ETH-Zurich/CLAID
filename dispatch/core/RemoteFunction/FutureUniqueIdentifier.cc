#include "FutureUniqueIdentifier.hh"

namespace claid {

std::mutex FutureUniqueIdentifier::mutex;
long FutureUniqueIdentifier::currentId = 0;

    FutureUniqueIdentifier::FutureUniqueIdentifier(std::string id) : identifier(id)
    {
    }

    FutureUniqueIdentifier FutureUniqueIdentifier::makeUniqueIdentifier()
    {
        mutex.lock();
        long idCopy = currentId;
        currentId++;
        mutex.unlock();

        std::string identifier = std::string("CLAID_CPP_") + std::to_string(idCopy) + std::string("_") +
             std::to_string(Time::now().toUnixTimestampMilliseconds());

        return FutureUniqueIdentifier(identifier);
    } 

    FutureUniqueIdentifier FutureUniqueIdentifier::fromString(std::string identifier)
    {
        return FutureUniqueIdentifier(identifier);
    }   

    std::string FutureUniqueIdentifier::toString() const
    {
        return this->identifier;
    }

    bool FutureUniqueIdentifier::operator <(const FutureUniqueIdentifier& rhs) const
    {
        return identifier < rhs.identifier;
    }

}