#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include <core/parser.h>
#include <core/exceptions.h>

#include <utils.h>

namespace Core {

    Parser::Parser() {}
    Parser::~Parser() {};

    // Reads a single byte as Int8 and advances the offset
    uint8_t Parser::readInt8(const uint8_t* buffer, uint16_t& offset) {
        uint8_t value = buffer[offset];
        offset += 1;
        return value;
    }

    // Reads a big-endian Int16 from the buffer and advances the offset
    uint16_t Parser::readInt16(const uint8_t* buffer, uint16_t& offset) {
        uint16_t value = (buffer[offset] << 8) | buffer[offset + 1];
        offset += 2;
        return value;
    }

    // Reads a big-endian Int32 from the buffer and advances the offset
    uint32_t Parser::readInt32(const uint8_t* buffer, uint16_t& offset) {
        uint32_t value =
            (buffer[offset] << 24) |
            (buffer[offset + 1] << 16) |
            (buffer[offset + 2] << 8) |
            buffer[offset + 3];
        offset += 4;
        return value;
    }

    // Reads a big-endian Int64 from the buffer and advances the offset
    uint64_t Parser::readInt64(const uint8_t* buffer, uint16_t& offset) {
        uint64_t value =
            ((uint64_t)buffer[offset] << 56) |
            ((uint64_t)buffer[offset + 1] << 48) |
            ((uint64_t)buffer[offset + 2] << 40) |
            ((uint64_t)buffer[offset + 3] << 32) |
            ((uint64_t)buffer[offset + 4] << 24) |
            ((uint64_t)buffer[offset + 5] << 16) |
            ((uint64_t)buffer[offset + 6] << 8) |
            (uint64_t)buffer[offset + 7];
        offset += 8;
        return value;
    }

    std::string Parser::getClientId(const uint8_t* buffer, uint16_t length) {
        return std::string(reinterpret_cast<const char*>(buffer + 14), length);
    }

    uint32_t Parser::getRequestSize(const uint8_t* buffer) {
        return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
    }

    uint32_t Parser::getCorrelationId(const uint8_t* buffer) {
        return (buffer[8] << 24) | (buffer[9] << 16) | (buffer[10] << 8) | buffer[11];
    }

    uint16_t Parser::getApiKey(const uint8_t* buffer) {
        return (buffer[4] << 8) | buffer[5];
    }

    uint16_t Parser::getApiVersion(const uint8_t* buffer) {
        uint16_t apiVersion = (buffer[6] << 8) | buffer[7];
        return (apiVersion <= 4) ? apiVersion : CoreTypes::API_VERSION_ERROR_CODE;
    }

    uint16_t Parser::getClientIdLength(const uint8_t* buffer) {
        return (buffer[12] << 8) | buffer[13];
    }

    std::set<std::string> Parser::getTopics(const uint8_t* buffer, uint16_t& offset) {

        std::set<std::string> results {};
        uint8_t arrayLength = buffer[offset++] - 1;

        auto parseArray = [&](uint16_t arrayLength, std::set<std::string>& saveTo) {
            while (arrayLength--) {
                uint16_t topicLength = buffer[offset++] - 1;

                saveTo.insert(std::string(reinterpret_cast<const char*>(buffer + offset), topicLength));
                offset += topicLength + 1; // Adding one for the empty tag at the end
            }
        };

        parseArray(arrayLength, results);

        return results;
    }

    uint32_t Parser::readVarUInt(const uint8_t* buffer, uint16_t& offset) {
        uint32_t value = 0;
        int shift = 0;
        uint8_t byte;

        while (true) {
            byte = buffer[offset++];
            value |= (byte & 0x7F) << shift;

            if ((byte & 0x80) == 0) break;

            shift += 7;
        }

        // Zigzag decoding
        return value;
    }

    std::string Parser::readUUID(const uint8_t* buffer, uint16_t& offset) {

        std::uint8_t cByte {};

        std::array<uint8_t, 16> uuidBytes;
        for (int i = 0; i < 16; ++i) {
            uuidBytes[i] = buffer[offset + i];
        }

        std::stringstream ss;
        ss << std::hex;
        ss.fill('0');

        for (int i = 0; i < 16; ++i) {
            ss.width(2);
            ss << static_cast   <int>(uuidBytes[i]);
            if (i == 3 || i == 5 || i == 7 || i == 9) ss << "-";
        }

        return ss.str();
    }


    bool Parser::readBoolean(const uint8_t* buffer, uint16_t& offset) {
        bool value = buffer[offset] != 0;
        offset += 1;
        return value;
    }

    std::string Parser::readCompactNullableString(const uint8_t* buffer, uint16_t& offset) {
        uint32_t length = readInt16(buffer, offset);

        // Compact Nullable String length is (length + 1). 0 means null.
        // If length is 0, the string is null. If it's > 0, the actual string length is length - 1.
        if (length == 0) {
            return ""; // Representing null or empty based on convention
        }
        std::string str(reinterpret_cast<const char*>(buffer + offset), length);
        offset += length;
        return str;
    }

    void Parser::getFetchTopics(CoreTypes::ParsedRequest& r, const uint8_t* buffer, uint16_t& offset) {

        r.replicaId = readInt32(buffer, offset);
        r.maxWaitMs = readInt32(buffer, offset);
        r.minBytes = readInt32(buffer, offset);
        r.maxBytesRequest = readInt32(buffer, offset);
        r.isolationLevel = readInt8(buffer, offset);
        r.sessionId = readInt32(buffer, offset);

        uint32_t num_topics = readInt16(buffer, offset) - 1;
        if (num_topics == 0) return;

        r.fetchTopics.resize(num_topics); // Resize the vector to hold the fetchTopics

        for (uint32_t i = 0; i < num_topics; ++i) {
            r.fetchTopics[i].topicUUID = readUUID(buffer, offset);
        }

    }

    CoreTypes::ParsedRequest Parser::parseRequest(const uint8_t* buffer) {

        CoreTypes::ParsedRequest r {};
        uint16_t offset {};

        // Parse Header using helper methods
        r.requestSize = readInt32(buffer, offset); // 4 bytes
        r.apiKey = readInt16(buffer, offset);     // 2 bytes
        r.apiVersion = readInt16(buffer, offset); // 2 bytes
        r.correlationId = readInt32(buffer, offset); // 4 bytes

        if (r.apiKey == CoreTypes::DESCRIBE_TOPIC_API) {
            uint16_t offset = 15 + getClientIdLength(buffer);
            r.topics = getTopics(buffer, offset);

        }
        else if (r.apiKey == CoreTypes::FETCH_API) {
            r.clientId = readCompactNullableString(buffer, offset);
            getFetchTopics(r, buffer, offset);
        }

        return r;

    }

}