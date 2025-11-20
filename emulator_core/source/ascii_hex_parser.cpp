#include "emulator_core/source/ascii_hex_parser.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

bool parseHexByte(const std::string& hexByte, uint8_t& value) {
    // Validate input: should be 1-2 characters, all hex digits
    if (hexByte.empty() || hexByte.length() > 2) {
        return false;
    }

    // Check all characters are valid hex digits
    for (char c : hexByte) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    // Convert to integer
    try {
        unsigned long result = std::stoul(hexByte, nullptr, 16);
        if (result > 0xFF) {
            return false;  // Value too large for uint8_t
        }
        value = static_cast<uint8_t>(result);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<uint8_t> parseAsciiHexString(const std::string& hexData) {
    std::vector<uint8_t> binary;
    std::istringstream stream(hexData);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Skip comment lines (optional - Busicom files don't have them)
        if (line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Parse hex byte
        uint8_t value;
        if (parseHexByte(line, value)) {
            binary.push_back(value);
        } else {
            // Invalid hex byte - return empty vector to signal error
            return std::vector<uint8_t>();
        }
    }

    return binary;
}

std::vector<uint8_t> parseAsciiHexFile(const std::string& filename) {
    // Open file
    std::ifstream file(filename);
    if (!file.is_open()) {
        return std::vector<uint8_t>();  // Empty vector on error
    }

    // Read entire file into string
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    // Parse using string parser
    return parseAsciiHexString(buffer.str());
}
