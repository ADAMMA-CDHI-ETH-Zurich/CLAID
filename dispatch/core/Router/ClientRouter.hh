#include "dispatch/core/Router/Router.hh"

namespace claid
{

    // Forwards a package to either the LocalRouter, ClientRouter or ServerRouter, depending on the target of the package.
    class ClientRouter final : public Router
    {
        public:

        ClientRouter()
        {
            
        }

        void routePackage(std::shared_ptr<DataPackage> dataPackage) override final
        {
            
        }
    };
}