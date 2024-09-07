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
        static TLSServerKeyStore onlyEncryption(const std::string& pathToServerCert, const std::string& pathToServerKey)
        {
            TLSServerKeyStore store;
            store.loadCertificate(pathToServerCert);
            store.loadKey(pathToServerKey);
            store.encryption = true;
            return store;
        }

        /**
         * @brief Creates a TLSServerKeyStore with mutual encryption and authentication. This requires all clients to identify themselves with a certificate.
         * @param pathToServerCert The path to the server certificate.
         * @param pathToServerKey The path to the server key.
         * @param pathToClientCert The path to the client certificate.
         * @return The TLSServerKeyStore with mutual encryption and authentication.
         */
        static TLSServerKeyStore encryptionAndAuthentication(
            const std::string& pathToServerCert,
            const std::string& pathToServerKey,
            const std::string& pathToClientCert
        )
        {
            TLSServerKeyStore store;
            store.loadCertificate(pathToServerCert);
            store.loadKey(pathToServerKey);
            store.clientCertificate = pathToClientCert;
            store.encryption = true;
            store.authentication = true;
            return store;
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
         * @return True if the certificate was loaded successfully, false otherwise.
         */
        bool loadCertificate(const std::string& path)
        {
            return readFile(path, serverCert);
        }

        /**
         * @brief Loads the server key from a file.
         * @param path The path to the file containing the server key.
         * @return True if the key was loaded successfully, false otherwise.
         */
        bool loadKey(const std::string& path)
        {
            return readFile(path, serverKey);
        }

        /**
         * @brief Loads the client certificate from a file.
         * @param path The path to the file containing the client certificate.
         * @return True if the certificate was loaded successfully, false otherwise.
         */
        bool loadClientCertificate(const std::string& path)
        {
            return readFile(path, clientCertificate);
        }

        /**
         * @brief Reads the contents of a file.
         * @param path The path to the file to read.
         * @return The contents of the file as a string.
         */
        bool readFile(const std::string& path, std::string& content)
        {
            std::ifstream file(path, std::ios::binary);
            if(!file.is_open())
            {
                Logger::logError("Failed to open file: \"%s\"", path.c_str());
                return false;
            }
            content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            return true;
        }
    };
}