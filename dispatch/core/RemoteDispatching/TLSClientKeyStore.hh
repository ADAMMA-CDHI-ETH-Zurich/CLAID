#pragma once

#include <string>
#include <fstream>
#include "dispatch/core/Logger/Logger.hh"
/**
 * @file TLSClientKeyStore.hh
 * @brief Defines the TLSClientKeyStore structure for storing TLS server credentials.
 */

/**
 * @namespace claid
 * @brief The main namespace for the CLAID framework.
 */

namespace claid {
    /**
     * @struct TLSClientKeyStore
     * @brief A structure to store TLS client credentials.
     */
    struct TLSClientKeyStore
    {
        /**
         * @brief The client certificate.
         */
        std::string clientCert;

        /**
         * @brief The client key.
         */
        std::string clientKey;

        /**
         * @brief The certificate of the server. Required for mutual TLS authentication.
         * @note This is optional, and only required for strict identity verification. 
         * 
         * When set, the client will verify the server's certificate against this certificate upon connection.
         * This ensures that the client only connects to a trusted server.
         * 
         */
        std::string serverCertificate;

        /**
         * @brief Creates a TLSClientKeyStore with only encryption. Everyone can create their own keys and connect, however the connection is encrypted.
         * @param pathToServerCert The path to the server certificate.
         * @param pathToServerKey The path to the server key.
         * @return The TLSClientKeyStore with only encryption.
         */
        static TLSClientKeyStore onlyEncryption(const std::string& pathToServerCert, const std::string& pathToServerKey)
        {
            TLSClientKeyStore store;
            store.loadCertificate(pathToServerCert);
            store.loadKey(pathToServerKey);
            store.encryption = true;
            return store;
        }

        /**
         * @brief Creates a TLSClientKeyStore with mutual encryption and authentication. This requires all clients to identify themselves with a certificate.
         * @param pathToClientCert The path to the client certificate.
         * @param pathToClientKey The path to the client key.
         * @param pathToServerCert The path to the server certificate.
         * @return The TLSClientKeyStore with mutual encryption and authentication.
         */
        
        static TLSClientKeyStore encryptionAndAuthentication(
            const std::string& pathToClientCert, 
            const std::string& pathToClientKey, 
            const std::string& pathToServerCert
        )
        {
            TLSClientKeyStore store;
            store.loadCertificate(pathToClientCert);
            store.loadKey(pathToClientKey);
            store.loadServerCertificate(pathToServerCert);
            store.encryption = true;
            store.authentication = true;
            return store;
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
            return readFile(path, clientCert);
        }

        /**
         * @brief Loads the server key from a file.
         * @param path The path to the file containing the server key.
         * @return True if the key was loaded successfully, false otherwise.
         */
        bool loadKey(const std::string& path)
        {
            return readFile(path, clientKey);
        }

        /**
         * @brief Loads the server certificate from a file.
         * @param path The path to the file containing the server certificate.
         * @return True if the certificate was loaded successfully, false otherwise.
         */
        bool loadServerCertificate(const std::string& path)
        {
            return readFile(path, serverCertificate);
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