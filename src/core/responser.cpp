#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>

#include <utils.h>

#include <core/responser.h>

namespace Core {

    Responser::Responser(std::vector<ApiVersion> apiVerArray) :
        correlationId {},
        throttleTime {},
        apiVersion {},
        code {},
        responseSize {},
        apiVerArray {apiVerArray}
    {}

    Responser::~Responser() {}

    bool Responser::isCleanCode() {return code == 0;}

    uint32_t Responser::getResponseSize() {
        uint32_t size = 7;  // Start with 8 bytes for fixed protocol fields

        if (isCleanCode()) size += sizeof(uint32_t);  // 4 bytes for API version
        if (!apiVerArray.empty()) size += (apiVerArray.size() * 6) + apiVerArray.size();

        return size + 1; // Add 1 byte for end marker
    }

    void Responser::addApiVersionArray(char* buffer, char*& ptr) {

        uint8_t apiCount = apiVerArray.size() + 1; // N + 1
        memcpy(ptr, &apiCount, sizeof(apiCount));
        ptr += sizeof(apiCount);

        for (ApiVersion& apiVersion : apiVerArray) {

            uint16_t networkApiKey = htons(apiVersion.apiKey);
            uint16_t networkMinVersion = htons(apiVersion.minVersion);
            uint16_t networkMaxVersion = htons(apiVersion.maxVersion);

            // API version info
            memcpy(ptr, &networkApiKey, sizeof(networkApiKey));
            ptr += sizeof(networkApiKey);

            memcpy(ptr, &networkMinVersion, sizeof(networkMinVersion));
            ptr += sizeof(networkMinVersion);

            memcpy(ptr, &networkMaxVersion, sizeof(networkMaxVersion));
            ptr += sizeof(networkMaxVersion);

            uint8_t emptyTaggedFields = 0;
            memcpy(ptr, &emptyTaggedFields, sizeof(emptyTaggedFields));
            ptr += sizeof(emptyTaggedFields);

        }

    }

    void Responser::sendResponse(uint16_t clientFd) {

        responseSize = getResponseSize();

        char buffer[sizeof(uint32_t) + responseSize];
        char* ptr = buffer;

        // Network byte order conversion for all values once
        uint32_t networkResponseSize = htonl(responseSize);
        uint32_t networkCorrelationId = htonl(correlationId);
        uint32_t networkThrottleTime = htonl(throttleTime);
        uint16_t networkCode = htons(code);

        // Add response size
        memcpy(ptr, &networkResponseSize, sizeof(networkResponseSize));
        ptr += sizeof(networkResponseSize);

        // Add correlation ID
        memcpy(ptr, &networkCorrelationId, sizeof(networkCorrelationId));
        ptr += sizeof(networkCorrelationId);

        // Error code
        memcpy(ptr, &networkCode, sizeof(networkCode));
        ptr += sizeof(networkCode);

        // Add api version array
        addApiVersionArray(buffer, ptr);

        memcpy(ptr, &networkThrottleTime, sizeof(throttleTime));
        ptr += sizeof(networkThrottleTime);

        uint8_t emptyTaggedFields1 = 0;
        memcpy(ptr, &emptyTaggedFields1, sizeof(emptyTaggedFields1));
        ptr += sizeof(emptyTaggedFields1);

        // Send the complete buffer
        send(clientFd, buffer, sizeof(uint32_t) + responseSize, 0);
    }

}