#ifndef SERVER_H
#define SERVER_H

#include <cstdint>
#include <core/parser.h>

namespace Core {

    constexpr const uint16_t BUFFER_SIZE = 1024;
    constexpr const uint16_t MIN_API_VERSION = 0;
    constexpr const uint16_t MAX_API_VERSION = 16;

    class Server {

        public:

            friend void startListener(Server* server);
            friend void handleClientConnection(Server* server, uint16_t clientFd);

            static Server* createServer();

            void handleResponse(const uint8_t* buffer, size_t bytesReceived, uint16_t clientFd);

            Server();
            ~Server();

        private:

            uint8_t serverFd;
            uint8_t reuse;
            uint8_t connectionBacklog;
            uint16_t port;

            uint16_t minApiVersion;
            uint16_t maxApiVersion;

            Parser* parser;
    };

    void startListener(Server* server);
    void handleClientConnection(Server* server, uint16_t clientFd);

}

#endif // SERVER_H