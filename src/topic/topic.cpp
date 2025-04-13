#include <topic/topic.h>

#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <unordered_map>
#include <set>

#include <utils.h>

#include <topic/parser/reader.h>
#include <topic/structs.h>


namespace Topics {

    Topic::Topic(std::string fileName)
    : fileName {fileName}, records{}, recordsLooked{}, recordsFound{}
    {}
    Topic::~Topic() {}

    std::unordered_map<std::string, std::vector<TopicStructs::Record*>> Topic::findTopics(
        const std::set<std::string>& topicsToFind
    ) {

        std::unordered_map<std::string, std::vector<TopicStructs::Record*>> topicsFound {};

        Reader reader = Reader(fileName);
        reader.findTopics(topicsToFind, topicsFound, recordHeaders, records);

        return topicsFound;
    }
}