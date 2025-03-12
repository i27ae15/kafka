#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>

#include <core/server.h>
#include <core/exceptions.h>
#include <core/parser.h>
#include <core/responser.h>

#include <utils.h>

namespace Core {

    void handleClientConnection(Server* server, uint16_t clientFd) {

        uint8_t buffer[BUFFER_SIZE];
        size_t bytesReceived {};

        while (true) {
            bytesReceived = recv(clientFd, buffer, BUFFER_SIZE, 0);
            if (bytesReceived <= 0) {
                PRINT_KILL("BREAKING CONNECTION");
                break;
            }
            (void)server->handleResponse(buffer, bytesReceived, clientFd);
        }

        close(clientFd);

    }

    void startListener(Server* server) {
        PRINT_SUCCESS("LISTEN STARTED, WAITING FOR CONNECTION ON : " + std::to_string(server->serverFd));
        struct sockaddr_in clientAddr {};
        while (true) {
            socklen_t clientAddrLen = sizeof(clientAddr);
            uint16_t clientFd = accept(server->serverFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);

            PRINT_SUCCESS("CONNECTED WITH CLIENT ON: " + std::to_string(clientFd));
            std::thread worker(handleClientConnection, server, clientFd);
            worker.detach();
        }

    }

    void Server::handleResponse(const uint8_t* buffer, size_t bytesReceived, uint16_t clientFd) {

        std::vector<CoreTypes::ApiVersion> apiVersionArray = std::vector<CoreTypes::ApiVersion>{
            CoreTypes::ApiVersion{parser->getApiKey(buffer), minApiVersion, maxApiVersion},
            CoreTypes::ApiVersion{CoreTypes::DESCRIBE_TOPIC_API, minApiVersion, maxApiVersion}
        };
        CoreTypes::ParsedRequest pRequest = parser->parseRequest(buffer);
        Responser responser = Responser(clientFd, apiVersionArray, pRequest);

        if (pRequest.apiVersion > maxApiVersion) {
            responser.apiVersion = 0;
            responser.code = CoreTypes::API_VERSION_ERROR_CODE;
        }

        (void)responser.sendResponse(clientFd);
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

    Server::Server() :
        serverFd {},
        reuse {},
        connectionBacklog {},
        port {9092},
        parser {new Parser()},
        minApiVersion {MIN_API_VERSION},
        maxApiVersion {MAX_API_VERSION}
        {}
    Server::~Server() {}

}