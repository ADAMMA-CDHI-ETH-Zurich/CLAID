#pragma once

#include <string>
#include <fstream>
#include "dispatch/core/Logger/Logger.hh"
/**
 * @file TLSServerKeyStore.hh
 * @brief Defines the TLSServerKeyStore structure for storing TLS server credentials.
 */

/**
 * @namespace claid
 * @brief The main namespace for the CLAID framework.
 */

#include "absl/status/status.h"
namespace claid {
    /**
     * @struct TLSServerKeyStore
     * @brief A structure to store TLS server credentials.
     */
    struct TLSServerKeyStore
    {
        /**
         * @brief The server certificate.
         */
        std::string serverCert;

        /**
         * @brief The server key.
         */
        std::string serverKey;

        /**
         * @brief The certificate of the client(s) (each client needs to have the same certificate). Required for mutual TLS authentication.
         * @note This is optional, and only required for strict identity verification. 
         * 
         * When set, the server will verify the client's certificate against this certificate upon connection.
         * This ensures that only clients with a valid certificate can connect to the server.
         * 
         */
        std::string clientCertificate;

        /**
         * @brief Creates a TLSServerKeyStore with only encryption. Everyone can create their own keys and connect, however the connection is encrypted.
         * @param pathToServerCert The path to the server certificate.
         * @param pathToServerKey The path to the server key.
         * @return The TLSServerKeyStore with only encryption.
         */
        static absl::Status onlyEncryption(
            const std::string& pathToServerCert,
            const std::string& pathToServerKey,
            TLSServerKeyStore& store
        )
        {
            store = TLSServerKeyStore();
            absl::Status status = store.loadCertificate(pathToServerCert);
            if (!status.ok()) {
                Logger::logError("Failed to load server certificate: %s", status.ToString().c_str());
                return status;
            }
            status = store.loadKey(pathToServerKey);
            if (!status.ok()) {
                Logger::logError("Failed to load server key: %s", status.ToString().c_str());
                return status;
            }
            store.encryption = true;
            return absl::OkStatus();
        }

        /**
         * @brief Creates a TLSServerKeyStore with mutual encryption and authentication. This requires all clients to identify themselves with a certificate.
         * @param pathToServerCert The path to the server certificate.
         * @param pathToServerKey The path to the server key.
         * @param pathToClientCert The path to the client certificate.
         * @return The TLSServerKeyStore with mutual encryption and authentication.
         */
        static absl::Status encryptionAndAuthentication(
            const std::string& pathToServerCert,
            const std::string& pathToServerKey,
            const std::string& pathToClientCert,
            TLSServerKeyStore& store
        )
        {
            store = TLSServerKeyStore();
            absl::Status status = store.loadCertificate(pathToServerCert);
            if (!status.ok()) {
                Logger::logError("Failed to load server certificate: %s", status.ToString().c_str());
                return status;
            }
            status = store.loadKey(pathToServerKey);
            if (!status.ok()) {
                Logger::logError("Failed to load server key: %s", status.ToString().c_str());
                return status;
            }
            status = store.loadClientCertificate(pathToClientCert);
            if (!status.ok()) {
                Logger::logError("Failed to load client certificate: %s", status.ToString().c_str());
                return status;
            }
            store.encryption = true;
            store.authentication = true;
            return absl::OkStatus();
        }

        /**
         * @brief Checks if the server requires authentication.
         * @return True if the server requires authentication, i.e., a client certificate is provided, false otherwise.
         */
        bool requiresAuthentication() const
        {
            return authentication;
        }

    private:

        bool encryption = false;
        bool authentication = false;

        /**
         * @brief Loads the server certificate from a file.
         * @param path The path to the file containing the server certificate.
         * @return absl::Status A status indicating whether the operation was successful.
         */
        absl::Status loadCertificate(const std::string& path)
        {
            return readFile(path, serverCert);
        }

        /**
         * @brief Loads the server key from a file.
         * @param path The path to the file containing the server key.
         * @return absl::Status A status indicating whether the operation was successful.
         */
        absl::Status loadKey(const std::string& path)
        {
            return readFile(path, serverKey);
        }

        /**
         * @brief Loads the client certificate from a file.
         * @param path The path to the file containing the client certificate.
         * @return absl::Status A status indicating whether the operation was successful.
         */
        absl::Status loadClientCertificate(const std::string& path)
        {
            return readFile(path, clientCertificate);
        }

        /**
         * @brief Reads the contents of a file.
         * @param path The path to the file to read.
         * @param content A buffer to store the content of the file.
         * @return absl::Status A status indicating whether the operation was successful.
         */
        absl::Status readFile(const std::string& path, std::string& content)
        {
            std::ifstream file(path, std::ios::binary);
            if(!file.is_open())
            {
                return absl::NotFoundError(absl::StrCat("Failed to open file: ", path));
            }
            content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            return absl::OkStatus();
        }
    };
}