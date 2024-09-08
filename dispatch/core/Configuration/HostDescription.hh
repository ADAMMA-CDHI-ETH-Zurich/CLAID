/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#pragma once

#include <string>
#include <map>
#include "dispatch/core/Configuration/UniqueKeyMap.hh"
#include "dispatch/core/RemoteDispatching/TLSServerKeyStore.hh"
#include "dispatch/core/RemoteDispatching/TLSClientKeyStore.hh"
#include "dispatch/proto/claidservice.pb.h"

using namespace claidservice;
namespace claid
{
    struct HostDescription
    {


        HostConfig hostConfig;

        HostDescription()
        {

        }

        HostDescription(const HostConfig& hostConfig) : hostConfig(hostConfig)
        {

        }

        std::string getHostname() const
        {
            return hostConfig.hostname();
        }

        bool isServer() const
        {
            return hostConfig.has_server_config();
        }


        bool isClient() const
        {
            return hostConfig.has_connect_to();
        }

        /**
         * @brief Retrieves the address of this host, when it acts as server.
         * 
         * This method returns the address specified in the server configuration.
         * 
         * @return const std::string& The address to connect to.
         */
        const std::string& getHostServerAddress() const
        {
            return hostConfig.server_config().host_server_address();
        }

        /**
         * @brief Retrieves the the name of the host that this host should connect to.
         * 
         * This method returns the address specified in the client configuration.
         * 
         * @return const std::string& The address to connect to.
         */
        const std::string& getConnectToAddress() const
        {
            return hostConfig.connect_to().host();
        }

        /**
         * @brief Checks if a security configuration for the server was specified in the config.
         * 
         * This method checks if the server configuration has TLS settings.
         * 
         * @return true if TLS server settings are available, false otherwise.
         */
        bool hasTLSServerSettings() const
        {
            return hostConfig.server_config().security_settings_case() != ServerConfig::SecuritySettingsCase::SECURITY_SETTINGS_NOT_SET;
        }
        /**
         * @brief Checks if a security configuration for the client was specified in the config.
         * 
         * This method checks if the client configuration has TLS settings.
         * 
         * @return true if TLS client settings are available, false otherwise.
         */
        bool hasTLSClientSettings() const
        {
            return hostConfig.connect_to().security_settings_case() != ClientConfig::SecuritySettingsCase::SECURITY_SETTINGS_NOT_SET;
        }

        /**
         * @brief Retrieves the TLS server key store for the host.
         * 
         * This method checks if TLS server settings are available and returns the appropriate
         * TLSServerKeyStore based on the security settings specified in the configuration.
         * 
         * @return absl::Status Indicates whether the operation was successful.
         * @param store The TLSServerKeyStore to store the result in.
         */
        absl::Status getTLSServerKeyStore(TLSServerKeyStore& store) const
        {
            if(!hasTLSServerSettings())
            {
                throw std::runtime_error("HostDescription::getTLSServerKeyStore: No TLS server settings available.");
            }

            if(hostConfig.server_config().security_settings_case() == ServerConfig::SecuritySettingsCase::kTls)
            {
                absl::Status status = TLSServerKeyStore::serverBasedAuthentication(
                    hostConfig.server_config().tls().server_public_certificate(),
                    hostConfig.server_config().tls().server_private_key(),
                    store
                );
                return status;
            }
            else if(hostConfig.server_config().security_settings_case() == ServerConfig::SecuritySettingsCase::kTlsEncryptionAndAuthentication)
            {
                absl::Status status = TLSServerKeyStore::mutualTLS(
                    hostConfig.server_config().mutual_tls().server_public_certificate(), 
                    hostConfig.server_config().mutual_tls().server_private_key(),
                    hostConfig.server_config().mutual_tls().client_public_certificate(),
                    store
                );
                return status;
            }
            else
            {
                return absl::Status(absl::StatusCode::kInvalidArgument, "HostDescription::getTLSServerKeyStore: No TLS server settings specified in configuration.");
            }
        }
        /**
         * @brief Retrieves the TLS server key store for the host.
         * 
         * This method checks if TLS server settings are available and returns the appropriate
         * TLSServerKeyStore based on the security settings specified in the configuration.
         * 
         * @return absl::Status Indicates whether the operation was successful.
         * @param store The TLSServerKeyStore to store the result in.
         */
        absl::Status getTLSClientKeyStore(TLSClientKeyStore& store) const
        {
            if(hostConfig.connect_to().security_settings_case() == ClientConfig::SecuritySettingsCase::kMutualTls)
            {
                absl::Status status = TLSClientKeyStore::onlyEncryption(
                    hostConfig.connect_to().mutual_tls().server_public_certificate(),
                    store
                );
                return status;
            }
            else if(hostConfig.connect_to().security_settings_case() == ClientConfig::SecuritySettingsCase::kTlsEncryptionAndAuthentication)
            {
                absl::Status status = TLSClientKeyStore::encryptionAndAuthentication(
                    hostConfig.connect_to().mutual_tls().client_public_certificate(), 
                    hostConfig.connect_to().mutual_tls().client_private_key(),
                    hostConfig.connect_to().mutual_tls().server_public_certificate(),
                    store
                );
                return status;
            }
            else 
            {
                return absl::Status(absl::StatusCode::kInvalidArgument, "HostDescription::getTLSClientKeyStore: No TLS client settings specified in configuration.");
            }
        }
    };

    typedef UniqueKeyMap<HostDescription> HostDescriptionMap;

}