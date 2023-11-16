#include "dispatch/core/CLAID.hh"
#include "dispatch/core/Router/RoutingQueueMergerGeneric.hh"
#include "dispatch/core/Router/MasterRouter.hh"
#include "dispatch/core/Configuration/Configuration.hh"
#include "dispatch/core/RemoteDispatching/HostUserTable.hh"
#include "dispatch/core/Logger/Logger.hh"


namespace claid {
    bool CLAID::started = false;
}