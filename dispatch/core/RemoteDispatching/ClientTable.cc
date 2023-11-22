#include "dispatch/core/RemoteDispatching/ClientTable.hh"

namespace claid {

    ClientTable::ClientTable()
    {

    }

    SharedQueue<DataPackage>& ClientTable::getQueue()
    {
        return toRemoteClientQueue;
    }
}