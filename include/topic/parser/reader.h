#ifndef TOPIC_PARSER_H
#define TOPIC_PARSER_H

#include <cstdint>
#include <fstream>
#include <string>

#include <topic/structs.h>


namespace Topics {

    class Reader {

        public:

        Reader(std::string fileName);
        ~Reader();

        void readFileAndSetRecords(
            std::vector<TopicStructs::RecordBatchHeader*>& recordHeaders,
            std::vector<std::vector<TopicStructs::Record*>>& records
        );

        private:

        std::ifstream file;
        std::string fileName;

        std::uint8_t cByte;

        // METHODS

        void openFile();

        template <typename T>
        void readValue(T& value) {
            static_assert(std::is_integral<T>::value, "Only integral types are supported");

            using UnsignedT = typename std::make_unsigned<T>::type;
            UnsignedT val = 0;
            constexpr size_t byteCount = sizeof(T);

            for (size_t i = 0; i < byteCount; ++i) {
                getNextByte();
                val = (val << 8) | cByte;
            }

            value = static_cast<T>(val);
        }

        std::uint8_t getNextByte();
        std::int32_t readVarint();
        std::string readUUID();

        void readRecords(
            uint32_t& numRecords,
            std::vector<std::vector<TopicStructs::Record*>>& records,
            std::uint32_t batchIdx
        );

        TopicStructs::BaseRecordValue* getRecordValue(std::uint32_t idx, std::uint32_t batchIdx);

        TopicStructs::RecordBatchHeader* readFullBatchHeader();

        TopicStructs::RecordFeatureLevelValue* readFeatureLevelRecord();
        TopicStructs::RecordTopicValue* readTopicRecordValue();
        TopicStructs::RecordPartitionValue* readPartitionRecordValue();
    };


}

#endif // TOPIC_PARSER_H
