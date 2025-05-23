#ifndef PARSER_H
#define PARSER_H

#include <cstdint>
#include <string>
#include <set>

#include <core/types.h>

namespace Core {

    // Example Kafka request:

    // 00 00 00 12    // Size: 18 bytes (excluding these 4 bytes)
    // 00 12          // API Key: 18 (API_VERSIONS)
    // 00 04          // API Version: 4
    // 12 34 56 78    // Correlation ID: 305419896
    // 00 05          // Client ID string length: 5 bytes
    // 6D 79 41 70 70 // Client ID: "myApp" in ASCII
    // 00             // Empty tagged fields

    class Parser {

        private:

        uint8_t readInt8(const uint8_t* buffer, uint16_t& offset);
        uint16_t readInt16(const uint8_t* buffer, uint16_t& offset);
        uint32_t readInt32(const uint8_t* buffer, uint16_t& offset);
        uint64_t readInt64(const uint8_t* buffer, uint16_t& offset);

        uint32_t readVarUInt(const uint8_t* buffer, uint16_t& offset);
        std::string readUUID(const uint8_t* buffer, uint16_t& offset);
        std::string readCompactNullableString(const uint8_t* buffer, uint16_t& offset);

        bool readBoolean(const uint8_t* buffer, uint16_t& offset);

        void getFetchTopics(CoreTypes::ParsedRequest& r, const uint8_t* buffer, uint16_t& offset);

        public:

        Parser();
        ~Parser();


        std::string getClientId(const uint8_t* buffer, uint16_t finishAt);

        uint32_t getRequestSize(const uint8_t* buffer);
        uint32_t getCorrelationId(const uint8_t* buffer);

        uint16_t getApiKey(const uint8_t* buffer);
        uint16_t getApiVersion(const uint8_t* buffer);

        uint16_t getClientIdLength(const uint8_t* buffer);

        std::set<std::string> getTopics(const uint8_t* buffer, uint16_t& offset);

        CoreTypes::ParsedRequest parseRequest(const uint8_t* buffer);
    };

}


#endif // PARSER_H