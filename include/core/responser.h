#ifndef RESPONSER_H
#define RESPONSER_H

#include <vector>

#include <cstdint>
#include <string>

#include <topic/structs.h>

#include <core/types.h>

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

    class Responser {

        public:

            Responser(
                uint16_t clientFd,
                std::vector<CoreTypes::ApiVersion> apiVerArray,
                CoreTypes::ParsedRequest pRequest
            );
            ~Responser();

            void sendResponse(uint16_t clientFd);

            bool isCleanCode();

            uint16_t clientFd;

            uint32_t correlationId;
            uint32_t throttleTime;
            uint32_t apiVersion;

            uint16_t code;

            uint32_t responseSize;

            std::vector<CoreTypes::ApiVersion> apiVerArray;
            CoreTypes::ParsedRequest pRequest;

            std::string requestBody;

        private:

            char* bufferPtr;

            void addEmptyTag();
            void addErrorCode();
            void addTopicsArray();
            void addThrottleTime();
            void addCorrelationId();
            void addApiVersionArray();
            void addResponseSize(char* buffer);

            void processKey18();
            void processKey75();
            void processKey1();

            void addPartitions(std::vector<TopicStructs::Record*> records);
            void addEmptyPartition();

            void writeUint8(uint8_t value);
            void writeUint16(uint16_t value);
            void writeUint32(uint32_t value);
            void writeString(const std::string& str);
            void writeBytes(const uint8_t* data, size_t length);
            void writeUUID(const std::array<uint8_t, CoreTypes::UUID_SIZE>& uuid);
            void writeBool(bool value);
            void writeVarInt(uint32_t value);
    };

}


#endif // RESPONSER_H