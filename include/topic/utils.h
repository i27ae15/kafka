#include <topic/structs.h>

namespace TopicUtils {

    void printRecord(TopicStructs::Record* record);
    void printRecordValue(TopicStructs::Record* record);
    void printBatchRecord(TopicStructs::RecordBatchHeader* recordBatch);

}