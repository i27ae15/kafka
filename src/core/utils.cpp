#include <iostream>
#include <iomanip>

namespace CoreUtils {

    void printBufferNoSize(const char* buffer, size_t length) {
        // Calculate the length excluding the first 4 bytes
        size_t displayLength = (length > 4) ? (length - 4) : 0;

        // Print header with both decimal and hex values
        std::cout << "Buffer contents [" << displayLength << " bytes (0x"
                  << std::hex << displayLength << std::dec << ")]:" << std::endl;

        // Print header row for column reference
        std::cout << "Offset  ";
        for (int i = 0; i < 16; i++) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << i << " ";
        }
        std::cout << "  ASCII" << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;

        // Print rows of 16 bytes each, starting from index 4
        for (size_t i = 4; i < length; i += 16) {
            // Print offset (adjusted to start from 0 for display purposes)
            std::cout << std::setw(6) << std::setfill('0') << std::hex << (i - 4) << "  ";

            std::string ascii = "";

            // Print hex values
            for (size_t j = 0; j < 16; j++) {
                if (i + j < length) {
                    // Convert to unsigned char to handle values > 127 correctly
                    unsigned char byte = static_cast<unsigned char>(buffer[i + j]);
                    std::cout << std::setw(2) << std::setfill('0') << std::hex
                              << static_cast<int>(byte) << " ";

                    // Collect printable ASCII characters for the right column
                    if (byte >= 32 && byte <= 126) {
                        ascii += static_cast<char>(byte);
                    } else {
                        ascii += ".";
                    }
                } else {
                    // Padding for incomplete rows
                    std::cout << "   ";
                    ascii += " ";
                }
            }

            // Print ASCII representation
            std::cout << "  " << ascii << std::endl;
        }

        std::cout << std::dec; // Reset to decimal format
    }

    void printBuffer(const char* buffer, size_t length) {
        std::cout << "Buffer contents [" << length << " bytes]:" << std::endl;

        // Print header row for column reference
        std::cout << "Offset  ";
        for (int i = 0; i < 16; i++) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << i << " ";
        }
        std::cout << "  ASCII" << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;

        // Print rows of 16 bytes each
        for (size_t i = 0; i < length; i += 16) {
            // Print offset
            std::cout << std::setw(6) << std::setfill('0') << std::hex << i << "  ";

            std::string ascii = "";

            // Print hex values
            for (size_t j = 0; j < 16; j++) {
                if (i + j < length) {
                    // Convert to unsigned char to handle values > 127 correctly
                    unsigned char byte = static_cast<unsigned char>(buffer[i + j]);
                    std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte) << " ";

                    // Collect printable ASCII characters for the right column
                    if (byte >= 32 && byte <= 126) {
                        ascii += static_cast<char>(byte);
                    } else {
                        ascii += ".";
                    }
                } else {
                    // Padding for incomplete rows
                    std::cout << "   ";
                    ascii += " ";
                }
            }

            // Print ASCII representation
            std::cout << "  " << ascii << std::endl;
        }

        std::cout << std::dec; // Reset to decimal format
    }

}