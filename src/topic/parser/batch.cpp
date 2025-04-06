#include <fstream>

#include <utils.h>

#include <topic/parser/reader.h>


namespace Topics {

    TopicStructs::RecordBatchHeader* Reader::readFullBatchHeader() {

        TopicStructs::RecordBatchHeader* batchRecord = new TopicStructs::RecordBatchHeader;

        readValue(batchRecord->baseOffset);
        readValue(batchRecord->batchLength);
        readValue(batchRecord->partitionEpoch);
        readValue(batchRecord->magicByte);
        readValue(batchRecord->crc);
        readValue(batchRecord->attributes);
        readValue(batchRecord->lastOffsetDelta);
        readValue(batchRecord->baseTimeStamp);
        readValue(batchRecord->maxTimeStamp);
        readValue(batchRecord->producerId);
        readValue(batchRecord->producerEpoch);
        readValue(batchRecord->baseSequence);
        readValue(batchRecord->recordsLength);

        return batchRecord;
    }
}