#include "dispatch/core/CLAID.hh"
using namespace claid;
class TestReceiverModule : public Module
{  
    private:
        Channel<std::string> channel;
        int ctr;
    public:
        void initialize(Properties properties)
        {
            Logger::logInfo("TestReceiverModule init");
            channel = subscribe<std::string>("DataChannel", &TestReceiverModule::onData, this);
        }

        void onData(ChannelData<std::string> data)
        {
            Logger::logInfo("Got data %s", data.getData().c_str());
        }
};

REGISTER_MODULE(TestReceiverModule, TestReceiverModule)