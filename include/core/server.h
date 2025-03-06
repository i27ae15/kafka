#ifndef SERVER_H
#define SERVER_H

#include <cstdint>

namespace Core {

    constexpr const int BUFFER_SIZE = 1024;

    class Server {

        public:

            friend void startListener(Server* server);

            static Server* createServer();

            void handleResponse(const uint8_t* buffer, size_t bytesReceived, uint16_t clientFd);

            Server();
            ~Server();

        private:

            uint8_t serverFd;
            uint8_t reuse;
            uint8_t connectionBacklog;
            uint16_t port;

    };

    void startListener(Server* server);

}

#endif // SERVER_H