#pragma once

namespace claid {
    enum RoutingDirection
    {
        // Route up in the routing tree (i.e., the host is a client of another server).
        ROUTE_UP,

        // Route down in the routing tree (i.e., the host is a server for another client).
        ROUTE_DOWN,

        // Route locally (i.e., to a Module running in the current instance of CLAID)
        ROUTE_LOCAL,
    };
}