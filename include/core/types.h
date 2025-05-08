#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <cstdint>
#include <vector>
#include <string>
#include <set>
#include <array>

namespace CoreTypes {

    constexpr const uint8_t API_VERSION_ERROR_CODE = 35;
    constexpr const uint8_t DESCRIBE_TOPIC_API = 75;
    constexpr const uint8_t REQUEST_API_VERSIONS = 18;
    constexpr const uint8_t FETCH_API = 1;
    constexpr const uint16_t UNKNOW_TOPIC_ERROR_CODE = 3;
    constexpr const uint16_t NO_ERROR = 0;
    constexpr const uint16_t UNKNOW_FETCH_TOPIC = 100;

    constexpr const uint8_t BYTE_SIZE = 1;
    constexpr const std::array<uint8_t, 16> NULL_UUID = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    constexpr const uint8_t NULL_PAGINATION_FIELD = 0xFF;
    constexpr const uint8_t UUID_SIZE = 16;
    constexpr const uint16_t MEGA_BYTE_SIZE = 1024;

    struct ApiVersion {
        uint16_t apiKey;
        uint16_t minVersion;
        uint16_t maxVersion;
    };

    // Structs to hold parsed data from the Fetch Request
    struct PartitionInfo {
        int32_t partition_index;
        int32_t current_leader_epoch;
        int64_t fetch_offset;
        int32_t last_fetched_epoch;
        int64_t log_start_offset;
        int32_t max_bytes;
        bool forget_decrypted_data;
        // Add fields for returned data/records if needed in later stages
    };

    struct TopicInfo {
        std::vector<uint8_t> topicId; // UUID is 16 bytes
        std::vector<PartitionInfo> partitions;
    };

    struct ParsedRequest {
        std::string client;

        uint16_t apiKey;
        uint16_t apiVersion;

        uint32_t requestSize;
        uint32_t correlationId;

        std::string clientId;

        std::set<std::string> topics;

        // Fetch Request specific fields (v16)
        int32_t replicaId;
        int32_t maxWaitMs;
        int32_t minBytes;
        int32_t maxBytesRequest;
        int8_t isolationLevel;
        int32_t sessionId;
        int32_t sessionEpoch;

        std::vector<TopicInfo> fetchTopics;
    };

}


#endif // CORE_TYPES_H