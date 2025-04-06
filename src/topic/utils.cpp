#include <utils.h>

#include <topic/structs.h>


namespace TopicUtils {

    void printBatchRecord(TopicStructs::RecordBatchHeader* recordBatch) {

        PRINT_SML_SEPARATION;
        PRINT_INFO("PRINTING BATCH");
        PRINT_HIGHLIGHT("BASE_OFFSET: " + std::to_string(recordBatch->baseOffset));
        PRINT_HIGHLIGHT("BATCH_LENGTH: " + std::to_string(recordBatch->batchLength));
        PRINT_HIGHLIGHT("PARTITION_EPOCH: " + std::to_string(recordBatch->partitionEpoch));
        PRINT_HIGHLIGHT("MAGIC_BYTE: " + std::to_string(recordBatch->magicByte));
        PRINT_HIGHLIGHT("CRC: " + std::to_string(recordBatch->crc));
        PRINT_HIGHLIGHT("ATTRIBUTES: " + std::to_string(recordBatch->attributes));
        PRINT_HIGHLIGHT("LAST_OFFSET_DELTA: " + std::to_string(recordBatch->lastOffsetDelta));
        PRINT_HIGHLIGHT("BASE_TIMESTAMP: " + std::to_string(recordBatch->baseTimeStamp));
        PRINT_HIGHLIGHT("MAX_TIMESTAMP: " + std::to_string(recordBatch->maxTimeStamp));
        PRINT_HIGHLIGHT("PRODUCER_ID: " + std::to_string(recordBatch->producerId));
        PRINT_HIGHLIGHT("PRODUCER_EPOCH: " + std::to_string(recordBatch->producerEpoch));
        PRINT_HIGHLIGHT("BASE_SEQUENCE: " + std::to_string(recordBatch->baseSequence));
        PRINT_HIGHLIGHT("RECORDS_LENGTH: " + std::to_string(recordBatch->recordsLength));
        PRINT_SML_SEPARATION;

    }

    void printRecord(TopicStructs::Record* record) {

        PRINT_SML_SEPARATION;
        PRINT_INFO("PRINTING RECORD");
        PRINT_HIGHLIGHT("RECORD_LENGTH: " + std::to_string(record->length));
        PRINT_HIGHLIGHT("RECORD_ATTRIBUTES: " + std::to_string(record->attributes));
        PRINT_HIGHLIGHT("TIMESTAMP_DELTA: " + std::to_string(record->timeStampDelta));
        PRINT_HIGHLIGHT("OFFSET_DELTA: " + std::to_string(record->offsetDelta));
        PRINT_HIGHLIGHT("KEY_LENGTH: " + std::to_string(record->keyLength));
        PRINT_HIGHLIGHT("KEY: " + std::to_string(record->key));
        PRINT_HIGHLIGHT("VALUE_LENGTH: " + std::to_string(record->valueLength));
        PRINT_HIGHLIGHT("HEADERS_ARRAY_COUNT: " + std::to_string(record->headersArrayCount));
        PRINT_SML_SEPARATION;

    }

    void printRecordValue(TopicStructs::Record* record) {
        PRINT_SML_SEPARATION;

        if (!record->recordValue) {
            PRINT_HIGHLIGHT("RECORD_VALUE: null");
            PRINT_SML_SEPARATION;
            return;
        }

        TopicStructs::BaseRecordValue* value = record->recordValue;

        PRINT_INFO("PRINTING RECORD_VALUE");
        PRINT_HIGHLIGHT("FRAME_VERSION: " + std::to_string(value->frameVersion));
        PRINT_HIGHLIGHT("TYPE: " + std::to_string(value->type));  // This is still stored in the object
        PRINT_HIGHLIGHT("VERSION: " + std::to_string(value->version));
        PRINT_HIGHLIGHT("NAME_LENGTH: " + std::to_string(value->nameLength));
        PRINT_HIGHLIGHT("NAME: " + value->name);

        if (auto* feature = dynamic_cast<TopicStructs::RecordFeatureLevelValue*>(value)) {
            PRINT_HIGHLIGHT("FEATURE_LEVEL: " + std::to_string(feature->featureLevel));
        }
        else if (auto* topic = dynamic_cast<TopicStructs::RecordTopicValue*>(value)) {
            PRINT_HIGHLIGHT("UUID: " + topic->uuid);
        }
        else if (auto* partition = dynamic_cast<TopicStructs::RecordPartitionValue*>(value)) {
            PRINT_HIGHLIGHT("PARTITION_ID: " + std::to_string(partition->partitionId));
            PRINT_HIGHLIGHT("TOPIC_UUID: " + partition->topicUUID);
            PRINT_HIGHLIGHT("REPLICA_ARRAY_LENGTH: " + std::to_string(partition->lengthOfReplicaArray));
            PRINT_HIGHLIGHT("REPLICA_ARRAY: " + std::to_string(partition->replicaArray));
            PRINT_HIGHLIGHT("IN_SYNC_REPLICA_ARRAY_LENGTH: " + std::to_string(partition->lengthOfInSyncReplicaArray));
            PRINT_HIGHLIGHT("IN_SYNC_REPLICA_ARRAY: " + std::to_string(partition->inSyncReplicaArray));
            PRINT_HIGHLIGHT("REMOVING_REPLICA_ARRAY_LENGTH: " + std::to_string(partition->lengthOfRemovingReplicaArray));
            PRINT_HIGHLIGHT("ADDING_REPLICAS_ARRAY_LENGTH: " + std::to_string(partition->lengthOfAddingReplicasArray));
            PRINT_HIGHLIGHT("LEADER: " + std::to_string(partition->leader));
            PRINT_HIGHLIGHT("LEADER_EPOCH: " + std::to_string(partition->leaderEpoch));
            PRINT_HIGHLIGHT("PARTITION_EPOCH: " + std::to_string(partition->partitionEpoch));
            PRINT_HIGHLIGHT("DIRECTORIES_ARRAY_LENGTH: " + std::to_string(partition->LengthOfDirectoriesArray));
            PRINT_HIGHLIGHT("DIRECTORIES_ARRAY_UUID: " + partition->directoriesArrayUUID);
            PRINT_HIGHLIGHT("TAGGED_FIELDS_COUNT: " + std::to_string(partition->taggedFieldsCount));
        } else {
            PRINT_HIGHLIGHT("Unknown RecordValueType – no extra fields printed.");
        }

        PRINT_HIGHLIGHT("TAGGED_FIELDS_COUNT: " + std::to_string(value->taggedFieldsCount));
        PRINT_SML_SEPARATION;
    }


}