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

        Topics::Topic topic = Topics::Topic();

        std::unordered_map<
            std::string, std::vector<TopicStructs::Record*>
        > topicsFound = topic.findTopics(pRequest.topics, TopicStructs::FindBy::NAME);

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

    void Responser::addTopicsFetch() {

        writeUint8(pRequest.fetchTopics.size() + 1);
        if (!pRequest.fetchTopics.size()) return;

        for (CoreTypes::TopicInfo topic : pRequest.fetchTopics) {
            std::array<uint8_t, 16> buuid = TopicUtils::parseUUIDToBytes(topic.topicUUID);
            writeUUID(buuid);
        }

        Topics::Topic topic = Topics::Topic();
        std::unordered_map<
            std::string, std::vector<TopicStructs::Record*>
        > topicsFound = topic.findTopics({pRequest.fetchTopics[0].topicUUID}, TopicStructs::FindBy::UUID);

        for (const CoreTypes::TopicInfo& topic : pRequest.fetchTopics) {
            bool topicExists = topicsFound.count(topic.topicUUID);
            std::uint16_t errorCode = topicExists ? CoreTypes::NO_ERROR : CoreTypes::UNKNOW_FETCH_TOPIC;
            // Momentous partitions
            // addEmptyTag();
            writeVarInt(2);
            writeUint32(0);
            writeUint16(errorCode);

            writeUint64(-1);     // high_watermark
            writeUint64(-1);     // last_stable_offset
            writeUint64(-1);     // log_start_offset
            writeVarInt(0 + 1); // diverging_epoch (empty compact array => VarInt(1))
            writeVarInt(0);     // snapshot_id (nullable struct, null => VarInt(0))
            writeVarInt(0 + 1); // aborted_transactions (empty compact array => VarInt(1))
            writeUint32(-1);     // preferred_read_replica
            writeVarInt(0);

            addEmptyTag();
            addEmptyTag();
        }
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

            case CoreTypes::REQUEST_API_VERSIONS:
                (void)processKey18();
                break;

            case CoreTypes::FETCH_API:
                (void)processKey1();
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

    void Responser::writeUint64(uint64_t value) {
        unsigned char* current_ptr = reinterpret_cast<unsigned char*>(bufferPtr);

        current_ptr[0] = (value >> 56) & 0xFF; // Most significant byte
        current_ptr[1] = (value >> 48) & 0xFF;
        current_ptr[2] = (value >> 40) & 0xFF;
        current_ptr[3] = (value >> 32) & 0xFF;
        current_ptr[4] = (value >> 24) & 0xFF;
        current_ptr[5] = (value >> 16) & 0xFF;
        current_ptr[6] = (value >> 8)  & 0xFF;
        current_ptr[7] = value & 0xFF;

        bufferPtr = reinterpret_cast<decltype(bufferPtr)>(current_ptr + sizeof(uint64_t));
        responseSize += sizeof(uint64_t);
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

    void Responser::writeVarInt(uint32_t value) {
        while ((value & ~0x7F) != 0) {
            writeUint8((value & 0x7F) | 0x80); // Set continuation bit
            value >>= 7;
        }
        writeUint8(value); // Last byte with MSB = 0
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

    void Responser::processKey1() {

        addEmptyTag();
        addThrottleTime();
        writeUint16(0); // ErrorCode
        writeUint32(0); // Session Id
        addTopicsFetch();
        writeVarInt(1); // VarInt(1)
        addEmptyTag();
    }

}