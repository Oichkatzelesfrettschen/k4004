#pragma once

#include <vector>
#include <string>
#include <cstdint>

/**
 * @file ascii_hex_parser.hpp
 * @brief Utility for parsing ASCII hex ROM files (Busicom format)
 *
 * The Busicom 141-PF ROM file format is ASCII hexadecimal with one byte per line.
 * This parser converts such files to binary format suitable for ROM loading.
 *
 * Example format:
 *   FE        <- Header
 *   FF        <- End of I/O masks
 *   F0        <- ROM data byte 0
 *   11        <- ROM data byte 1
 *   ...
 *
 * Usage:
 *   std::vector<uint8_t> binary = parseAsciiHexFile("busicom_141-PF.obj");
 *   rom.load(binary.data(), binary.size());
 */

/**
 * Parse an ASCII hex file into binary data
 *
 * @param filename Path to ASCII hex file
 * @return Vector of bytes (empty on error)
 */
std::vector<uint8_t> parseAsciiHexFile(const std::string& filename);

/**
 * Parse ASCII hex from string buffer
 *
 * @param hexData String containing hex data (newline delimited)
 * @return Vector of bytes (empty on error)
 */
std::vector<uint8_t> parseAsciiHexString(const std::string& hexData);

/**
 * Convert ASCII hex byte string to binary
 *
 * @param hexByte String like "FE" or "0A"
 * @param value Output parameter for converted value
 * @return true if conversion successful, false otherwise
 */
bool parseHexByte(const std::string& hexByte, uint8_t& value);
