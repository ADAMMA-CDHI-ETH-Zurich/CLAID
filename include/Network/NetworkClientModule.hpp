#pragma once

#include "RemoteConnection/ConnectionLink.hpp"
#include "RemoteConnection/RemoteModule/RemoteModule.hpp"
#include "RemoteConnection/RemoteConnectedEntity.hpp"
#include "RemoteConnection/Error/ErrorRemoteRuntimeOutOfSync.hpp"
#include "RemoteConnection/Error/ErrorConnectionTimeout.hpp"
#include "Network/SocketConnectionModule.hpp"
#include "Network/NetworkModule.hpp"

namespace claid
{
    namespace Network
    {
        class NetworkClientModule : public NetworkModule
        {

            public:
                NetworkClientModule()
                {
                }

                NetworkClientModule(std::string ip, int port)
                {
                    this->address = ip + std::string(":") + std::to_string(port);
                }


            template<typename Reflector>
            void reflect(Reflector& r)
            {
                REFLECT_BASE(r, NetworkModule)
                r.member("ConnectTo", this->address, "");
                r.member("TimeoutMsWhenTryingToConnect", this->timeoutInMs, "How long to wait for a response when (trying to) connect to a server.", static_cast<size_t>(3));
                r.member("TryToReconnectAfterMs", this->tryToReconnectAfterMs, "If the connection is lost (or could not be established), after how many ms should we try to reconnect? Set to 0 if reconnect should not happen.", static_cast<size_t>(1500));
            }

            private:
                std::string address;
                size_t timeoutInMs = 3;
                size_t tryToReconnectAfterMs = 200;

                bool connected = false;
                bool disabled = false;

                RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity = nullptr;

                Channel<RemoteConnection::Error> errorChannel;
                
                void getIPAndPortFromAddress(const std::string& address, std::string& ip, int& port);
                void initialize();

                bool connectToServer();
                void onConnectedSuccessfully(SocketClient socketClient);

                void onErrorReceived(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity, ChannelData<RemoteConnection::Error> error);

                template<typename T>
                void callError()
                {
                    RemoteConnection::Error error;
                    error.set<T>();
                    
                    // Make sure to call onError asynchronous. That way, we avoid recursive calls of onError.
                    // (E.g.: If error is ErrorConnectToAdressFailed, then we might try to reconnect. If that fails again,
                    // it would call onError again, and so on and so forth resulting in unbound recursion, which would eventually lead
                    // to a stack overflow).
                    this->callInModuleThread<NetworkClientModule, RemoteConnection::RemoteConnectedEntity*, RemoteConnection::Error>(&NetworkClientModule::onError, this, this->remoteConnectedEntity, error);
                }

                void onError(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity, RemoteConnection::Error error);

                void onConnectionLost(RemoteConnection::RemoteConnectedEntity* remoteConnectedEntity);
                void tryToReconnect();

                void onNetworkStateChangeRequested(const NetworkStateChangeRequest& networkStateChangeRequest);
        };

    }
}