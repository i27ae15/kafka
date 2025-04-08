#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <unordered_map>
#include <array>

#include <utils.h>

#include <topic/topic.h>
#include <topic/structs.h>
#include <topic/utils.h>

#include <core/responser.h>
#include <core/types.h>

namespace Core {

    Responser::Responser(uint16_t clientFd, std::vector<CoreTypes::ApiVersion> apiVerArray, CoreTypes::ParsedRequest pRequest) :
        correlationId {},
        throttleTime {},
        apiVersion {},
        code {},
        responseSize {},
        apiVerArray {apiVerArray},
        clientFd {},
        bufferPtr {},
        pRequest {pRequest}
    {}

    Responser::~Responser() {}

    bool Responser::isCleanCode() {return code == 0;}

    void Responser::addApiVersionArray() {

        uint8_t apiCount = apiVerArray.size() + 1; // N + 1
        memcpy(bufferPtr, &apiCount, CoreTypes::BYTE_SIZE);
        bufferPtr += CoreTypes::BYTE_SIZE;
        responseSize++;

        for (CoreTypes::ApiVersion& apiVersion : apiVerArray) {

            uint16_t networkApiKey = htons(apiVersion.apiKey);
            uint16_t networkMinVersion = htons(apiVersion.minVersion);
            uint16_t networkMaxVersion = htons(apiVersion.maxVersion);

            // API version info
            memcpy(bufferPtr, &networkApiKey, CoreTypes::BYTE_SIZE * 2);
            bufferPtr += CoreTypes::BYTE_SIZE * 2;

            memcpy(bufferPtr, &networkMinVersion, CoreTypes::BYTE_SIZE * 2);
            bufferPtr += CoreTypes::BYTE_SIZE * 2;

            memcpy(bufferPtr, &networkMaxVersion, CoreTypes::BYTE_SIZE * 2);
            bufferPtr += CoreTypes::BYTE_SIZE * 2;

            responseSize += 6;

            addEmptyTag();
        }

    }

    void Responser::addTopicsArray() {
        uint8_t arrayLength = pRequest.topics.size() + 1;

        std::string topicName = *pRequest.topics.begin();
        PRINT_HIGHLIGHT("REQUESTED TOPIC: " + topicName  + " | SIZE: " + std::to_string(topicName.size()));

        // Read the file
        Topics::Topic topic = Topics::Topic();

        // std::vector<TopicStructs::RecordBatchHeader*> records = topic.getRecords();
        std::unordered_map<
            std::string, std::vector<TopicStructs::Record*>
        > topicsFound = topic.findTopics(pRequest.topics);

        memcpy(bufferPtr, &arrayLength, CoreTypes::BYTE_SIZE);
        bufferPtr += CoreTypes::BYTE_SIZE;

        responseSize++;

        for (std::string topic : pRequest.topics) {

            // add unknow topic error code for the moment

            bool topicExists = topicsFound.count(topic);

            std::uint16_t errorCode = topicExists ? CoreTypes::NO_ERROR : CoreTypes::UNKNOW_TOPIC_ERROR_CODE;
            uint16_t networkErrorCode = htons(errorCode);
            memcpy(bufferPtr, &networkErrorCode, CoreTypes::BYTE_SIZE * 2);
            bufferPtr += CoreTypes::BYTE_SIZE * 2;
            responseSize += CoreTypes::BYTE_SIZE * 2;

            uint8_t topicLength = topic.size() + 1;
            memcpy(bufferPtr, &topicLength, CoreTypes::BYTE_SIZE);
            bufferPtr += CoreTypes::BYTE_SIZE;
            responseSize++;

            memcpy(bufferPtr, topic.c_str(), topic.size());
            bufferPtr += topic.size();
            responseSize += topic.size();

            std::array<uint8_t, CoreTypes::UUID_SIZE> uuid = CoreTypes::NULL_UUID;
            if (topicExists) {
                auto* topicValue = dynamic_cast<TopicStructs::RecordTopicValue*>(topicsFound[topic][0]->recordValue);
                uuid = TopicUtils::parseUUIDToBytes(topicValue->uuid);
            }

            memcpy(bufferPtr, uuid.data(), CoreTypes::UUID_SIZE);
            bufferPtr += CoreTypes::UUID_SIZE;
            responseSize += CoreTypes::UUID_SIZE;

            // To indicate if is internal
            addEmptyTag();

            uint8_t partitionArray = 1;
            if (topicExists) partitionArray++;
            memcpy(bufferPtr, &partitionArray, CoreTypes::BYTE_SIZE);
            bufferPtr += CoreTypes::BYTE_SIZE;
            responseSize++;

            uint32_t authorizeOperations = htonl(0x00000df8);
            memcpy(bufferPtr, &authorizeOperations, CoreTypes::BYTE_SIZE * 4);
            bufferPtr += CoreTypes::BYTE_SIZE * 4;
            responseSize += 4;

            addEmptyTag();

        }

        // Add null byte;
        memcpy(bufferPtr, &CoreTypes::NULL_PAGINATION_FIELD, CoreTypes::BYTE_SIZE);
        bufferPtr += CoreTypes::BYTE_SIZE;
        responseSize++;
    }

    void Responser::addThrottleTime() {

        uint32_t networkThrottleTime = htonl(throttleTime);

        memcpy(bufferPtr, &networkThrottleTime, sizeof(throttleTime));
        bufferPtr += sizeof(networkThrottleTime);

        responseSize += 4;
    }

    void Responser::addCorrelationId() {

        // Network byte order conversion for all values once
        uint32_t networkCorrelationId = htonl(pRequest.correlationId);

        // Add correlation ID
        memcpy(bufferPtr, &networkCorrelationId, sizeof(networkCorrelationId));
        bufferPtr += sizeof(networkCorrelationId);

        responseSize += 4;
    }

    void Responser::addErrorCode() {

        uint16_t networkCode = htons(code);

        memcpy(bufferPtr, &networkCode, sizeof(networkCode));
        bufferPtr += sizeof(networkCode);

        responseSize += 2;
    }

    void Responser::addEmptyTag() {

        uint8_t emptyTaggedFields1 = 0;

        memcpy(bufferPtr, &emptyTaggedFields1, sizeof(emptyTaggedFields1));
        bufferPtr += sizeof(emptyTaggedFields1);

        responseSize++;
    }

    void Responser::addResponseSize(char* buffer) {

        bufferPtr = buffer;
        uint32_t networkResponseSize = htonl(responseSize);

        memcpy(bufferPtr, &networkResponseSize, sizeof(networkResponseSize));

    }

    void Responser::sendResponse(uint16_t clientFd) {

        char buffer[sizeof(uint32_t) + CoreTypes::MEGA_BYTE_SIZE];
        bufferPtr = buffer + 4; // Reserved first four bytes to add response size

        addCorrelationId();

        switch (pRequest.apiKey) {
            case CoreTypes::DESCRIBE_TOPIC_API:
                (void)processKey75();
                break;

            case CoreTypes::NORMAL_API:
                (void)processKey18();
                break;

            default:
                break;
        }

        addEmptyTag();

        addResponseSize(buffer);
        send(clientFd, buffer, sizeof(uint32_t) + responseSize, 0);

    }

    void Responser::processKey18() {
        addErrorCode();
        addApiVersionArray();
        addThrottleTime();
    }

    void Responser::processKey75() {
        addEmptyTag();
        addThrottleTime();
        addTopicsArray();
    }

}