#include <core/parser.h>

namespace Core {

    constexpr const int API_VERSION_ERROR_CODE = 35;

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
        return (apiVersion <= 4) ? apiVersion : API_VERSION_ERROR_CODE;
    }

    uint16_t Parser::getClientIdLength(const uint8_t* buffer) {
        return (buffer[12] << 8) | buffer[13];
    }

}