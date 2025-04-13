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
        writeUint8(apiVerArray.size() + 1); // apiCount = N + 1

        for (CoreTypes::ApiVersion& apiVersion : apiVerArray) {
            writeUint16(apiVersion.apiKey);
            writeUint16(apiVersion.minVersion);
            writeUint16(apiVersion.maxVersion);
            addEmptyTag();
        }
    }

    void Responser::addTopicsArray() {

        std::string topicName = *pRequest.topics.begin();
        // PRINT_HIGHLIGHT("REQUESTED TOPIC: " + topicName + " | SIZE: " + std::to_string(topicName.size()));

        Topics::Topic topic = Topics::Topic();

        std::unordered_map<
            std::string, std::vector<TopicStructs::Record*>
        > topicsFound = topic.findTopics(pRequest.topics);

        writeUint8(pRequest.topics.size() + 1); // arrayLength

        for (const std::string& topic : pRequest.topics) {
            bool topicExists = topicsFound.count(topic);

            std::uint16_t errorCode = topicExists ? CoreTypes::NO_ERROR : CoreTypes::UNKNOW_TOPIC_ERROR_CODE;
            writeUint16(errorCode);

            writeString(topic);

            std::array<uint8_t, CoreTypes::UUID_SIZE> uuid = CoreTypes::NULL_UUID;
            if (topicExists) {
                auto* topicValue = dynamic_cast<TopicStructs::RecordTopicValue*>(topicsFound[topic][0]->recordValue);
                uuid = TopicUtils::parseUUIDToBytes(topicValue->uuid);
            }

            writeUUID(uuid);

            addEmptyTag(); // isInternal = False
            topicExists ? addPartitions(topicsFound[topic]) : addEmptyPartition();
        }

        writeUint8(CoreTypes::NULL_PAGINATION_FIELD); // Add null byte
        addEmptyTag();
    }


    void Responser::addPartitions(std::vector<TopicStructs::Record*> records) {

        writeUint8(records.size());

        for (int i {1}; i < records.size(); i++) {

            auto* partitionRecord = dynamic_cast<TopicStructs::RecordPartitionValue*>(records[i]->recordValue);
            writeUint16(CoreTypes::NO_ERROR);
            writeUint32(i - 1);
            writeUint32(partitionRecord->leader);
            writeUint32(partitionRecord->leaderEpoch);

            // Replica nodes
            writeUint8(2);
            writeUint32(0x01);

            // ISR array
            writeUint8(2);
            writeUint32(0x01);

            // Eligible leader replicas (compact array: 0 elements encoded as 0x01)
            writeUint8(0x01);

            // Last Known ELR (compact array: empty, so encoded as 0x01)
            writeUint8(0x01);

            // Offline replicas (compact array: empty, so encoded as 0x01)
            writeUint8(0x01);

            addEmptyTag();

        }

        writeUint32(0x00000df8); // authorizeOperations
        addEmptyTag();

    }

    void Responser::addEmptyPartition() {
        writeUint8(1);                    // partitionArrayLength
        writeUint32(0x00000df8);         // authorizeOperations
        addEmptyTag();
    }

    void Responser::addThrottleTime() {
        writeUint32(throttleTime);
    }

    void Responser::addCorrelationId() {
        writeUint32(pRequest.correlationId);
    }

    void Responser::addErrorCode() {
        writeUint16(code);
    }

    void Responser::addEmptyTag() {
        writeUint8(0x00); // emptyTaggedFields1
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

    // uint8_t
    void Responser::writeUint8(uint8_t value) {
        *bufferPtr = value;
        bufferPtr += sizeof(uint8_t);
        responseSize += sizeof(uint8_t);
    }

    // uint16_t
    void Responser::writeUint16(uint16_t value) {
        uint16_t net = htons(value);
        memcpy(bufferPtr, &net, sizeof(net));
        bufferPtr += sizeof(net);
        responseSize += sizeof(net);
    }

    // uint32_t
    void Responser::writeUint32(uint32_t value) {
        uint32_t net = htonl(value);
        memcpy(bufferPtr, &net, sizeof(net));
        bufferPtr += sizeof(net);
        responseSize += sizeof(net);
    }

    void Responser::writeString(const std::string& str) {
        writeUint8(static_cast<uint8_t>(str.size() + 1)); // +1 for null terminator or Kafka-style strings
        memcpy(bufferPtr, str.c_str(), str.size());
        bufferPtr += str.size();
        responseSize += str.size();
    }

    void Responser::writeBytes(const uint8_t* data, size_t length) {
        memcpy(bufferPtr, data, length);
        bufferPtr += length;
        responseSize += length;
    }

    void Responser::writeUUID(const std::array<uint8_t, CoreTypes::UUID_SIZE>& uuid) {
        writeBytes(uuid.data(), CoreTypes::UUID_SIZE);
    }

    void Responser::writeBool(bool value) {
        uint8_t byte = value ? 1 : 0;
        writeUint8(byte);
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