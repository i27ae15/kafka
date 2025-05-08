#include <vector>
#include <array>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <set>

#include <utils.h>

#include <topic/parser/reader.h>
#include <topic/structs.h>
#include <topic/utils.h>


namespace Topics {

    Reader::Reader(std::string fileName) : fileName{fileName}, file{}, cByte{} {}
    Reader::~Reader() {}

    void Reader::openFile() {
        file.open(fileName, std::ios::binary);
        if (!file) throw std::runtime_error("NO FILE WITH NAME " + fileName);
    }

    // Utility methods

    std::uint8_t Reader::getNextByte() {

        if (file.read(reinterpret_cast<char*>(&cByte), sizeof(cByte))) {
            return cByte;
        } else {
            throw std::runtime_error("END OF FILE REACHED");
        }

    }

    std::string Reader::readUUID() {
        std::array<uint8_t, 16> uuidBytes;
        for (int i = 0; i < 16; ++i) {
            readValue(cByte);
            uuidBytes[i] = cByte;
        }

        std::stringstream ss;
        ss << std::hex;
        ss.fill('0');

        for (int i = 0; i < 16; ++i) {
            ss.width(2);
            ss << static_cast   <int>(uuidBytes[i]);
            if (i == 3 || i == 5 || i == 7 || i == 9) ss << "-";
        }

        return ss.str();
    }

    int32_t Reader::readVarint() {
        int32_t value = 0;
        int shift = 0;
        uint8_t byte;

        while (true) {
            byte = getNextByte();
            value |= (byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) break;
            shift += 7;
        }

        // Zigzag decoding
        return (value >> 1) ^ -(value & 1);
    }

    // Methods For Records

    void Reader::findTopics(
        const std::set<std::string>& topicsToFind,
        TopicStructs::FindBy findBy,
        std::unordered_map<std::string, std::vector<TopicStructs::Record*>>& topics,
        std::vector<TopicStructs::RecordBatchHeader*>& recordHeaders,
        std::vector<std::vector<TopicStructs::Record*>>& records
    ) {

        size_t topicsFound {};
        size_t batchIdx {};

        (void)openFile();

        while(topicsToFind.size() > topicsFound) {

            try {

                TopicStructs::RecordBatchHeader* batchRecord = readFullBatchHeader();
                // TopicUtils::printBatchRecord(batchRecord);

                recordHeaders.push_back(batchRecord);
                records.push_back({});
                readRecords(batchRecord->recordsLength, records, batchIdx);

                batchIdx++;
                if (batchIdx == 1) continue;

                TopicStructs::BaseRecordValue* value = records[batchIdx - 1][0]->recordValue;

                if (findBy == TopicStructs::FindBy::UUID) {
                    if (auto* topicValue = dynamic_cast<TopicStructs::RecordTopicValue*>(value)) {
                        std::string& uuid = topicValue->uuid;

                        if (topicsToFind.count(uuid)) {
                            topics[uuid] = records[batchIdx - 1];
                            topicsFound++;
                        }
                    }
                }
                else if (findBy == TopicStructs::FindBy::NAME) {
                    std::string& topicName = records[batchIdx - 1][0]->recordValue->name;

                    if (topicsToFind.count(topicName)) {
                        topics[topicName] = records[batchIdx - 1];
                        topicsFound++;
                    }
                }

            } catch (std::runtime_error) {
                PRINT_INFO("ENTIRE FILE READ, NOT ALL TOPICS FOUND");
                return;
            }

        }

        PRINT_SUCCESS("ALL TOPICS FOUND");
    }
}