#ifndef CORE_TOPICS_H
#define CORE_TOPICS_H

#include <vector>
#include <string>
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <set>

#include <topic/structs.h>


namespace Topics {

    inline constexpr const char* defaultFileName = "/tmp/kraft-combined-logs/__cluster_metadata-0/00000000000000000000.log";

    class Topic {

        public:

        Topic(std::string fileName = defaultFileName);
        ~Topic();

        std::vector<TopicStructs::RecordBatchHeader*> getRecords();
        std::unordered_map<std::string, std::vector<TopicStructs::Record*>> findTopics(
            const std::set<std::string>& recordsToFind
        );

        private:

        std::unordered_map<std::string, std::vector<TopicStructs::Record*>> recordsFound;

        std::vector<TopicStructs::RecordBatchHeader*> recordHeaders;

        // where
        // rSize = recordHeaders.size()
        // records[i] -> (N) own by recordHeader
        std::vector<std::vector<TopicStructs::Record*>> records;


        bool recordsLooked;

        std::string fileName;
        std::vector<std::string> topics;


    };

}

#endif // CORE_TOPICS_H
