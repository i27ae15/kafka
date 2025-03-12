#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <cstdint>
#include <vector>
#include <string>

namespace CoreTypes {

    constexpr const uint8_t API_VERSION_ERROR_CODE = 35;
    constexpr const uint8_t DESCRIBE_TOPIC_API = 75;
    constexpr const uint8_t NORMAL_API = 18;
    constexpr const uint16_t UNKNOW_TOPIC_ERROR_CODE = 3;

    constexpr const uint8_t BYTE_SIZE = 1;
    constexpr const uint8_t NULL_UUID[16] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    constexpr const uint8_t NULL_PAGINATION_FIELD = 0xFF;
    constexpr const uint8_t NULL_UUID_SIZE = 16;
    constexpr const uint16_t MEGA_BYTE_SIZE = 1024;

    struct ApiVersion {
        uint16_t apiKey;
        uint16_t minVersion;
        uint16_t maxVersion;
    };

    struct ParsedRequest {
        std::string client;

        uint16_t apiKey;
        uint16_t apiVersion;

        uint32_t requestSize;
        uint32_t correlationId;

        std::vector<std::string> topics;
    };

}


#endif // CORE_TYPES_H