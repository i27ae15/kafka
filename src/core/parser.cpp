#include <vector>
#include <string>
#include <iostream>

#include <core/parser.h>
#include <core/exceptions.h>

#include <utils.h>

namespace Core {

    Parser::Parser() {}
    Parser::~Parser() {};

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

    std::vector<std::string> Parser::getTopics(const uint8_t* buffer, uint16_t& offset) {

        std::vector<std::string> results {};
        uint8_t arrayLength = buffer[offset++] - 1;

        auto parseArray = [&](uint16_t arrayLength, std::vector<std::string>& saveTo) {
            while (arrayLength--) {
                uint16_t topicLength = buffer[offset++] - 1;

                saveTo.emplace_back(reinterpret_cast<const char*>(buffer + offset), topicLength);
                offset += topicLength;
            }
        };

        parseArray(arrayLength, results);

        return results;
    }

    CoreTypes::ParsedRequest Parser::parseRequest(const uint8_t* buffer) {

        CoreTypes::ParsedRequest r {};

        r.requestSize = getRequestSize(buffer); // 4
        r.apiKey = getApiKey(buffer); // 2
        r.apiVersion = getApiVersion(buffer); // 2
        r.correlationId = getCorrelationId(buffer); // 4

        uint16_t offset = 15 + getClientIdLength(buffer);


        if (r.apiKey == CoreTypes::DESCRIBE_TOPIC_API) r.topics = getTopics(buffer, offset);
        return r;

    }

}