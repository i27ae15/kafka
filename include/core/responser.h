#ifndef RESPONSER_H
#define RESPONSER_H

#include <vector>

#include <cstdint>
#include <string>

namespace Core {

    // Example response

    // Length (4 bytes)
    // Correlation ID (4 bytes)
    // Error Code (2 bytes)
    // API Count (4 bytes)
    // [For each API:
    // - API Key (2 bytes)
    // - Min Version (2 bytes)
    // - Max Version (2 bytes)
    // - Tagged Fields (1+ bytes, variable length)
    // ]
    // Throttle Time (4 bytes) [moved to end in version 3+]
    // Tagged Fields (1+ bytes, variable length)

    constexpr const uint8_t UNSUPPORTED_API_ERROR_CODE = 35;

    struct ApiVersion {
        uint16_t apiKey;
        uint16_t minVersion;
        uint16_t maxVersion;
    };

    class Responser {

        public:

            Responser(std::vector<ApiVersion> apiVerArray);
            ~Responser();

            void sendResponse(uint16_t clientFd);

            bool isCleanCode();

            uint32_t correlationId;
            uint32_t throttleTime;
            uint32_t apiVersion;

            uint16_t code;

            uint32_t responseSize;

            std::vector<ApiVersion> apiVerArray;

            std::string requestBody;

        private:

        /**
         * @brief Calculates the total size of a Kafka protocol response message.
         *
         * @details This function computes the total byte size of a Kafka protocol response
         * message. The calculation starts with 8 bytes for fixed protocol fields
         * (typically for message size and request API key), then adds the size of various
         * response-specific fields.
         *
         * Response structure includes:
         * - 4 bytes for correlation ID (matches the ID from the request)
         * - 2 bytes for response error code
         * - 1 byte for API versions count
         * - Conditionally 4 bytes for API version if the error code is clean
         * - 2 bytes for API key
         * - 2 bytes for minimum supported version
         * - 2 bytes for maximum supported version
         * - 1 byte for tagged field
         * - 1 byte for additional end marker
         *
         * @return The total size of the response message in bytes
         */
        uint32_t getResponseSize();

        void addApiVersionArray(char* buffer, char*& ptr);

    };

}


#endif // RESPONSER_H