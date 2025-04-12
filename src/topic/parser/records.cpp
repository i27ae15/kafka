#include <vector>

#include <utils.h>

#include <topic/structs.h>
#include <topic/parser/reader.h>
#include <topic/utils.h>


namespace Topics {

    TopicStructs::RecordFeatureLevelValue* Reader::readFeatureLevelRecord() {

        TopicStructs::RecordFeatureLevelValue* recordVal = new TopicStructs::RecordFeatureLevelValue;

        readValue(recordVal->frameVersion);
        readValue(recordVal->type);
        readValue(recordVal->version);
        readValue(recordVal->nameLength);

        recordVal->name.clear();
        for (uint8_t i = {}; i < recordVal->nameLength; ++i) {
            readValue(cByte);
            recordVal->name.push_back(static_cast<char>(cByte));
        }
        recordVal->featureLevel = readVarint();
        readValue(recordVal->taggedFieldsCount);

        return recordVal;

    }

    TopicStructs::RecordTopicValue* Reader::readTopicRecordValue() {

        TopicStructs::RecordTopicValue* recordVal = new TopicStructs::RecordTopicValue;

        readValue(recordVal->frameVersion);
        readValue(recordVal->type);
        readValue(recordVal->version);
        readValue(recordVal->nameLength);

        recordVal->name.clear();
        for (uint8_t i = {}; i < recordVal->nameLength - 1; ++i) {
            readValue(cByte);
            recordVal->name.push_back(static_cast<char>(cByte));
        }
        recordVal->uuid = readUUID();
        readValue(recordVal->taggedFieldsCount);

        return recordVal;

    }

    TopicStructs::RecordPartitionValue* Reader::readPartitionRecordValue() {

        TopicStructs::RecordPartitionValue* partitionRecord = new TopicStructs::RecordPartitionValue();

        readValue(partitionRecord->frameVersion);
        readValue(partitionRecord->type);
        readValue(partitionRecord->type); // Still duplicated as noted
        readValue(partitionRecord->version);
        readValue(partitionRecord->partitionId);
        // partitionRecord->partitionId = readVarint();

        partitionRecord->topicUUID = readUUID();

        readValue(partitionRecord->lengthOfReplicaArray);
        readValue(partitionRecord->replicaArray);

        readValue(partitionRecord->lengthOfInSyncReplicaArray);
        readValue(partitionRecord->inSyncReplicaArray);

        readValue(partitionRecord->lengthOfRemovingReplicaArray);
        readValue(partitionRecord->lengthOfAddingReplicasArray);

        readValue(partitionRecord->leader);
        readValue(partitionRecord->leaderEpoch);
        readValue(partitionRecord->partitionEpoch);
        readValue(partitionRecord->LengthOfDirectoriesArray);

        partitionRecord->directoriesArrayUUID = readUUID();
        readValue(partitionRecord->taggedFieldsCount);

        return partitionRecord;
    }

    void Reader::readRecords(
        uint32_t& numRecords,
        std::vector<std::vector<TopicStructs::Record*>>& records,
        std::uint32_t batchIdx
    ) {

        for (uint32_t i {}; i < numRecords; i++) {
            TopicStructs::Record* record = new TopicStructs::Record();

            readValue(record->length);
            readValue(record->attributes);
            readValue(record->timeStampDelta);
            readValue(record->offsetDelta);
            record->keyLength = readVarint();

            if (record->keyLength > 0) readValue(record->key);
            readValue(record->valueLength);

            record->recordValue = getRecordValue(i, batchIdx);

            readValue(record->headersArrayCount);
            records[batchIdx].push_back(record);

            // TopicUtils::printRecord(record);
            // TopicUtils::printRecordValue(record);

        }
    }

    TopicStructs::BaseRecordValue* Reader::getRecordValue(std::uint32_t idx, std::uint32_t batchIdx) {

        if (batchIdx == 0 && idx == 0) return readFeatureLevelRecord();
        if (batchIdx == 1 && idx == 0) return readTopicRecordValue();

        return readPartitionRecordValue();
    }

}