#pragma once

#include <string>
#include <fstream>
#include "dispatch/core/Logger/Logger.hh"
/**
 * @file TLSClientKeyStore.hh
 * @brief Defines the TLSClientKeyStore structure for storing TLS client credentials.
 */

namespace claid {
    /**
     * @struct TLSClientKeyStore
     * @brief A structure to store TLS client credentials.
     */
    struct TLSClientKeyStore
    {

        /**
         * @brief The certificate of the server. For server based authentication (typical TLS case), this is all that's needed.
         * 
         * When set, the client will verify the server's certificate against this certificate upon connection.
         * This ensures that the client only connects to a trusted server.
         * 
         */
        std::string serverCertificate;

        /**
         * @brief The client certificate. Only required for mutual TLS authentication.
         * 
         * When set, the client will present this certificate to the server upon connection.
         * This ensures that the server knows the client's identity. The server will 
         * verify the client's certificate against it's known certificates.
         * This requires that the server has the client's certificate in it's known certificates.
         * 
         * @note This is optional, and only required for mutual TLS. However, if it is provided,
         * then the client private key is also required. Also, it requires that the server has set up mutual TLS as well!
         */
        std::string clientPublicCert;

        /**
         * @brief The client private key. Only required for mutual TLS authentication.
         * 
         * When set, the client will use this key to encrypt it's certificate, and then sends the certificate to the server.
         * The server will then try to decrypt the certificate using it's known client certificate.
         * This means that the server needs to have the SAME client certificate as the client.
         * If the server decrypts the certificate successfully, the client is authenticated, i.e., the certificate matches.
         * 
         * @note This is optional, and only required for mutual TLS. However, if it is provided,
         * then the client public certificate is also required. Also, it requires that the server has set up mutual TLS as well!
         */
        std::string clientPrivateKey;

   

        /**
         * @brief Creates a TLSClientKeyStore with only encryption. Everyone can create their own keys and connect, however the connection is encrypted.
         * @param pathToServerCert The path to the server certificate.
         * @param pathToServerKey The path to the server key.
         * @return absl::Status A status indicating whether the operation was successful.
         */
        static absl::Status serverBasedAuthentication(
            const std::string& pathToServerCert,
            TLSClientKeyStore& store
        )
        {
            store = TLSClientKeyStore();
            absl::Status status = store.loadServerCertificate(pathToServerCert);
            if (!status.ok()) {
                Logger::logError("Failed to load server certificate: %s", status.ToString().c_str());
                return status;
            }
        
            store.mutualTLSRequired = false;
            return absl::OkStatus();
        }

        /**
         * @brief Creates a TLSClientKeyStore with mutual encryption and authentication. This requires all clients to identify themselves with a certificate.
         * @param pathToClientCert The path to the client certificate.
         * @param pathToClientKey The path to the client key.
         * @param pathToServerCert The path to the server certificate.
         * @return absl::Status A status indicating whether the operation was successful.
         */
        
        static absl::Status mutualTLS(
            const std::string& pathToClientCert, 
            const std::string& pathToClientKey, 
            const std::string& pathToServerCert,
            TLSClientKeyStore& store    
        )
        {
            store = TLSClientKeyStore();
            absl::Status status = store.loadServerCertificate(pathToClientCert);
            if (!status.ok()) {
                Logger::logError("Failed to load client certificate: %s", status.ToString().c_str());
                return status;
            }
            status = store.loadClientPrivateKey(pathToClientKey);
            if (!status.ok()) {
                Logger::logError("Failed to load client key: %s", status.ToString().c_str());
                return status;
            }
            status = store.loadClientPublicCert(pathToServerCert);
            if (!status.ok()) {
                Logger::logError("Failed to load server certificate: %s", status.ToString().c_str());
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
        absl::Status loadServerCertificate(const std::string& path)
        {
            return readFile(path, serverCertificate);
        }

        /**
         * @brief Loads the server key from a file.
         * @param path The path to the file containing the server key.
         * @return absl::Status A status indicating whether the operation was successful.
         */
        absl::Status loadClientPrivateKey(const std::string& path)
        {
            return readFile(path, clientPrivateKey);
        }

        /**
         * @brief Loads the server certificate from a file.
         * @param path The path to the file containing the server certificate.
         * @return absl::Status A status indicating whether the operation was successful.
         */
        absl::Status loadClientPublicCert(const std::string& path)
        {
            return readFile(path, clientPublicCert);
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