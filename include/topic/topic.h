#ifndef CORE_TOPICS_H
#define CORE_TOPICS_H

#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

#include <topic/structs.h>


namespace Topics {

    inline constexpr const char* defaultFileName = "/tmp/kraft-combined-logs/__cluster_metadata-0/00000000000000000000.log";

    class Topic {

        public:

        Topic(std::string fileName = defaultFileName);
        ~Topic();

        std::vector<TopicStructs::RecordBatchHeader*> getRecords();

        private:

        std::vector<TopicStructs::RecordBatchHeader*> recordHeaders;
        std::vector<std::vector<TopicStructs::Record*>> records;

        // where
        // rSize = recordHeaders.size()
        // records[i] -> (N) own by recordHeader


        bool recordsLooked;

        std::uint8_t getNextByte();
        std::string fileName;

        std::vector<std::string> topics;

    };

}

#endif // CORE_TOPICS_H
