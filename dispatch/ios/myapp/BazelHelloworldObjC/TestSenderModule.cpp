#include "dispatch/core/CLAID.hh"
using namespace claid;
class TestSenderModule : public Module
{  
    private:
        Channel<std::string> channel;
        int ctr;
    public:
        void initialize(Properties properties)
        {
            Logger::logInfo("TestSenderModule init");
            channel = publish<std::string>("DataChannel");
            registerPeriodicFunction("PeriodicFunction", &TestSenderModule::periodicFunction, this, Duration::seconds(1));
        }

        void periodicFunction()
        {
            Logger::logInfo("Posting data!");
            std::string data = "Test " + std::to_string(ctr);
            channel.post(data);
            ctr++;
        }
};

REGISTER_MODULE(TestSenderModule, TestSenderModule)