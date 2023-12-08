#include "dispatch/core/RemoteDispatching/ClientTable.hh"

namespace claid {

    ClientTable::ClientTable()
    {

    }

    SharedQueue<DataPackage>& ClientTable::getFromRemoteClientQueue()
    {
        return this->fromRemoteClientQueue;
    }

    SharedQueue<DataPackage>& ClientTable::getToRemoteClientQueue()
    {
        return this->toRemoteClientQueue;
    }
}