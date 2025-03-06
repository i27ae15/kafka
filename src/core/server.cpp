#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <core/server.h>
#include <core/exceptions.h>

#include <utils.h>

namespace Core {

    int32_t generateCorrelationId() {
        return static_cast<int32_t>(rand());  // Generate a random 32-bit signed integer
    }

    void sendCorrelationId(int clientFd) {
        int32_t correlationId = generateCorrelationId();

        // Convert to big-endian (Kafka uses big-endian format)
        int32_t networkOrderId = htonl(correlationId);

        // Send the 4-byte integer in binary form
        send(clientFd, &networkOrderId, sizeof(networkOrderId), 0);
    }

    void sendKafkaResponse(int clientFd) {
        int32_t correlationId = 7; // generateCorrelationId();

        int32_t networkCorrelationId = htonl(correlationId);
        int32_t responseSize = htonl(sizeof(networkCorrelationId)); // 4 bytes for correlation_id

        char buffer[sizeof(responseSize) + sizeof(networkCorrelationId)];
        memcpy(buffer, &responseSize, sizeof(responseSize));
        memcpy(buffer + sizeof(responseSize), &networkCorrelationId, sizeof(networkCorrelationId));

        send(clientFd, buffer, sizeof(buffer), 0);
    }

    void startListener(Server* server) {
        PRINT_SUCCESS("LISTEN STARTED, WAITING FOR CONNECTION ON : " + std::to_string(server->serverFd));

        struct sockaddr_in clientAddr {};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientFd = accept(server->serverFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);


        uint8_t buffer[BUFFER_SIZE];
        size_t bytesReceived {};

        while (true) {
            bytesReceived = recv(clientFd, buffer, BUFFER_SIZE, 0);
            if (bytesReceived <= 0) {
                PRINT_ERROR("BREAKING CONNECTION");
                break;
            }
            (void)server->handleResponse(buffer, bytesReceived, clientFd);
        }

        close(clientFd);
    }

    void Server::handleResponse(const uint8_t* buffer, size_t bytesReceived, uint16_t clientFd) {

        // (void)sendCorrelationId(clientFd);
        (void)sendKafkaResponse(clientFd);

        // std::string toResponse = "7";
        // (void)send(clientFd, toResponse.c_str(), toResponse.size(), 0);
    }

    Server* Server::createServer() {

        Server* server = new Server();

        int _serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (_serverFd < 0) ServerException::SocketConnFailed();
        server->serverFd = _serverFd;

        int reuse = 1;
        if (setsockopt(server->serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            close(server->serverFd);
            throw ServerException::SetSockOptConnFailed();
        }

        struct sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(server->port);

        if (bind(server->serverFd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
            throw ServerException::BindToPortFailed(server->port);
        }

        if (listen(server->serverFd, server->connectionBacklog) != 0) {
            (void)close(server->serverFd);
            throw ServerException::ListenFailed();
        }

        PRINT_SUCCESS("SERVER INITIATED ON PORT : " + std::to_string(server->port));
        return server;
    }

    Server::Server() : serverFd{}, reuse{}, connectionBacklog{}, port{9092} {}
    Server::~Server() {}

}