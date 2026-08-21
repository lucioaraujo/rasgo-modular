#pragma once

#include "Graph.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <type_traits>

namespace rasgo::modular {

struct ArchivePackage {
    std::string manifest;
    std::string provenance;
    std::vector<std::uint8_t> payload;
};

class ArchiveStorage {
public:
    // Storage is deliberately a preparation/offline operation. It must not be
    // called from MultimodalChain::processAt() or another realtime callback.
    static bool writeEnvelope(const PayloadEnvelope& envelope,
                              const std::filesystem::path& directory) {
        if (!envelope.valid())
            return false;

        std::vector<std::uint8_t> payloadBytes;
        encodePayload(envelope.payload, payloadBytes);

        std::error_code error;
        std::filesystem::create_directories(directory, error);
        if (error)
            return false;

        const auto payloadPath = directory / "payload.bin";
        std::ofstream payloadFile(payloadPath, std::ios::binary | std::ios::trunc);
        if (!payloadFile)
            return false;
        if (!payloadBytes.empty())
            payloadFile.write(reinterpret_cast<const char*>(payloadBytes.data()),
                              static_cast<std::streamsize>(payloadBytes.size()));
        payloadFile.close();
        if (!payloadFile)
            return false;

        const auto checksum = checksumHex(payloadBytes);
        std::ofstream manifestFile(directory / "manifest.json", std::ios::trunc);
        if (!manifestFile)
            return false;
        manifestFile << "{\"archive_format\":\"rasgo-payload-envelope\","
                     << "\"format_version\":1,"
                     << "\"payload_file\":\"payload.bin\","
                     << "\"payload_size\":" << payloadBytes.size() << ","
                     << "\"payload_checksum\":\"fnv1a64-" << checksum << "\","
                     << "\"envelope\":" << envelope.toJson() << "}\n";
        manifestFile.close();
        if (!manifestFile)
            return false;

        std::ofstream provenanceFile(directory / "provenance.json", std::ios::trunc);
        if (!provenanceFile)
            return false;
        provenanceFile << "{\"source\":\"" << escapeJson(envelope.provenance.source)
                       << "\",\"transform\":\""
                       << escapeJson(envelope.provenance.transform)
                       << "\",\"version\":\""
                       << escapeJson(envelope.provenance.version)
                       << "\",\"license\":\""
                       << escapeJson(envelope.provenance.license) << "\"}\n";
        provenanceFile.close();
        return static_cast<bool>(provenanceFile);
    }

    static bool readPackage(const std::filesystem::path& directory,
                            ArchivePackage& package) {
        package = {};
        if (!readText(directory / "manifest.json", package.manifest)
            || !readText(directory / "provenance.json", package.provenance)
            || !readBinary(directory / "payload.bin", package.payload))
            return false;

        constexpr const char* checksumKey = "\"payload_checksum\":\"fnv1a64-";
        const auto checksumStart = package.manifest.find(checksumKey);
        if (checksumStart == std::string::npos)
            return false;
        const auto valueStart = checksumStart + std::char_traits<char>::length(checksumKey);
        const auto valueEnd = package.manifest.find('"', valueStart);
        if (valueEnd == std::string::npos)
            return false;
        const auto expected = package.manifest.substr(valueStart, valueEnd - valueStart);
        return expected == checksumHex(package.payload);
    }

    static std::string checksumHex(const std::vector<std::uint8_t>& bytes) {
        // FNV-1a is an integrity marker, not a cryptographic signature.
        std::uint64_t hash = 14695981039346656037ull;
        for (const auto byte : bytes) {
            hash ^= byte;
            hash *= 1099511628211ull;
        }
        std::ostringstream result;
        result << std::hex << std::setw(16) << std::setfill('0') << hash;
        return result.str();
    }

private:
    static bool readText(const std::filesystem::path& path, std::string& output) {
        std::ifstream file(path, std::ios::in);
        if (!file)
            return false;
        output.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        return static_cast<bool>(file) || file.eof();
    }

    static bool readBinary(const std::filesystem::path& path,
                           std::vector<std::uint8_t>& output) {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;
        output.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        return static_cast<bool>(file) || file.eof();
    }

    template <typename Value>
    static void appendValue(const Value& value, std::vector<std::uint8_t>& bytes) {
        static_assert(std::is_trivially_copyable_v<Value>);
        const auto* begin = reinterpret_cast<const std::uint8_t*>(&value);
        bytes.insert(bytes.end(), begin, begin + sizeof(Value));
    }

    static void appendString(const std::string& value,
                             std::vector<std::uint8_t>& bytes) {
        const auto size = static_cast<std::uint32_t>(value.size());
        appendValue(size, bytes);
        bytes.insert(bytes.end(), value.begin(), value.end());
    }

    static void encodePayload(const MultimodalPayload& payload,
                              std::vector<std::uint8_t>& bytes) {
        const auto category = static_cast<std::uint8_t>(payloadCategory(payload));
        appendValue(category, bytes);
        std::visit([&bytes](const auto& value) {
            using Value = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Value, AudioBlock>) {
                appendValue(value.sampleRate(), bytes);
                const auto channels = static_cast<std::uint32_t>(value.channels());
                const auto frames = static_cast<std::uint32_t>(value.frames());
                appendValue(channels, bytes);
                appendValue(frames, bytes);
                for (std::size_t channel = 0; channel < value.channels(); ++channel)
                    for (std::size_t frame = 0; frame < value.frames(); ++frame)
                        appendValue(value.at(channel, frame), bytes);
            } else if constexpr (std::is_same_v<Value, ControlBlock>) {
                const auto size = static_cast<std::uint32_t>(value.size());
                appendValue(size, bytes);
                for (std::size_t index = 0; index < value.size(); ++index)
                    appendValue(value.at(index), bytes);
            } else if constexpr (std::is_same_v<Value, DescriptorBlock>) {
                const auto width = static_cast<std::uint32_t>(value.width());
                const auto frames = static_cast<std::uint32_t>(value.frames());
                appendValue(width, bytes);
                appendValue(frames, bytes);
                for (std::size_t frame = 0; frame < value.frames(); ++frame)
                    for (std::size_t column = 0; column < value.width(); ++column)
                        appendValue(value.at(column, frame), bytes);
            } else {
                auto events = value;
                Event event;
                while (events.pop(event)) {
                    appendString(event.type, bytes);
                    appendValue(event.value, bytes);
                    appendValue(event.timestamp, bytes);
                }
            }
        }, payload);
    }
};

} // namespace rasgo::modular
