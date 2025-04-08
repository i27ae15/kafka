#include <topic/structs.h>

namespace TopicUtils {

    void printRecord(TopicStructs::Record* record);
    void printRecordValue(TopicStructs::Record* record);
    void printBatchRecord(TopicStructs::RecordBatchHeader* recordBatch);

    std::array<uint8_t, 16> parseUUIDToBytes(const std::string& uuidStr);

}