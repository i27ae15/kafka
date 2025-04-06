#include <topic/topic.h>

#include <fstream>
#include <stdexcept>
#include <cstdint>

#include <utils.h>

#include <topic/parser/reader.h>
#include <topic/structs.h>


namespace Topics {

    Topic::Topic(std::string fileName)
    : fileName {fileName}, records{}, recordsLooked{}
    {}
    Topic::~Topic() {}

    std::vector<TopicStructs::RecordBatchHeader*> Topic::getRecords() {

        if (recordsLooked) return recordHeaders;

        Reader reader = Reader(fileName);
        reader.readFileAndSetRecords(recordHeaders, records);

        recordsLooked = true;
        return recordHeaders;
    }

}