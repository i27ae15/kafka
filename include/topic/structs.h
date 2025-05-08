#ifndef TOPIC_STRUCTS_H
#define TOPIC_STRUCTS_H

#include <cstdint>
#include <string>


namespace TopicStructs {

    enum class RecordValueType {
        Feature,
        Topic,
        Partition,
    };

    enum class FindBy {
        NAME,
        UUID,
    };

    struct RecordBatchHeader {

        // As Written is the order that the information appears on the logs file
        uint64_t baseOffset;
        uint32_t batchLength;
        uint32_t partitionEpoch;
        uint8_t magicByte;
        uint32_t crc;
        uint16_t attributes;
        uint32_t lastOffsetDelta;
        uint64_t baseTimeStamp;
        uint64_t maxTimeStamp;
        int64_t producerId;
        int16_t producerEpoch;
        int32_t baseSequence;
        uint32_t recordsLength;

    };

    struct BaseRecordValue {

        virtual ~BaseRecordValue() = default;

        uint8_t frameVersion;
        uint8_t type;
        uint8_t version;
        uint8_t nameLength;
        std::string name;
        uint8_t taggedFieldsCount;

    };

    struct RecordFeatureLevelValue : public BaseRecordValue {

        uint16_t featureLevel;

    };

    struct RecordTopicValue : public BaseRecordValue {

        std::string uuid;

    };

    struct RecordPartitionValue : public BaseRecordValue {

        std::uint32_t partitionId;
        std::string topicUUID;

        uint8_t lengthOfReplicaArray;
        uint32_t replicaArray;

        uint8_t lengthOfInSyncReplicaArray;
        uint32_t inSyncReplicaArray;

        uint8_t lengthOfRemovingReplicaArray;
        uint8_t lengthOfAddingReplicasArray;
        uint32_t leader;
        uint32_t leaderEpoch;
        uint32_t partitionEpoch;
        uint8_t LengthOfDirectoriesArray;

        std::string directoriesArrayUUID;
        uint8_t taggedFieldsCount;

    };

    struct Record {

        uint8_t length;
        uint8_t attributes;
        uint8_t timeStampDelta;
        uint8_t offsetDelta;
        int8_t keyLength;
        uint8_t key;
        uint8_t valueLength;

        BaseRecordValue* recordValue;

        uint8_t headersArrayCount;

    };


}


#endif // TOPIC_STRUCTS_Hz`