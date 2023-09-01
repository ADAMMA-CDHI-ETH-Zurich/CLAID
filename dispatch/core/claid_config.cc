#include "dispatch/core/claid_config.hh"


namespace claid
{
    shared_ptr<ClaidConfig> parseConfigFile(const std::string& configFile) 
    {
        return make_shared<ClaidConfig>();
    }
}
