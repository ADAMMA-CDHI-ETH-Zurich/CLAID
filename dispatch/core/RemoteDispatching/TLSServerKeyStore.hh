#pragma once

#include <string>
#include <fstream>
#include "dispatch/core/Logger/Logger.hh"
/**
 * @file TLSServerKeyStore.hh
 * @brief Defines the TLSServerKeyStore structure for storing TLS server credentials.
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
         * @brief The server certificate, containing the public key.
         */
        std::string serverCert;

        /**
         * @brief The server private key.
         */
        std::string serverKey;

        /**
         * @brief The certificate of the client(s) (each client needs to have the same certificate). Required for mutual TLS authentication.
         * 
         * When set, the server will verify the client's certificate against this certificate upon connection.
         * Specifically, in the case of mutal TLS, the client has a private key and a public certificate.
         * It encrypts the public certificate with it's private key, and sends it to the server.
         * The server then decrypts the certificate using this certificate. If the decryption is successful,
         * the server knows that the client has a valid certificate.
         * 
         * @note This is optional, and only required for mutual TLS. 
         * It requires that the client has set up mutual TLS as well!
         */
        std::string clientCertificate;

        /**
         * @brief Creates a TLSServerKeyStore with only encryption. Everyone can create their own keys and connect, however the connection is encrypted.
         * @param pathToServerCert The path to the server certificate.
         * @param pathToServerKey The path to the server key.
         * @return The TLSServerKeyStore with only encryption.
         */
        static absl::Status serverBasedAuthentication(
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
            store.mutualTLSRequired = false;
            return absl::OkStatus();
        }

        /**
         * @brief Creates a TLSServerKeyStore with mutual encryption and authentication. This requires all clients to identify themselves with a certificate.
         * @param pathToServerCert The path to the server certificate.
         * @param pathToServerKey The path to the server key.
         * @param pathToClientCert The path to the client certificate.
         * @return The TLSServerKeyStore with mutual encryption and authentication.
         */
        static absl::Status mutualTLS(
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
            store.mutualTLSRequired = true;
            return absl::OkStatus();
        }

        /**
         * @brief Checks if mutual TLS is required.
         * @return True if mutual TLS is required, false otherwise.
         */
        bool requiresMutualTLS() const
        {
            return mutualTLSRequired;
        }

    private:

        bool mutualTLSRequired = false;

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