#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace rasgo::modular {

enum class PortKind {
    Audio,
    Control,
    Event,
};

inline const char* portKindName(const PortKind kind) {
    switch (kind) {
    case PortKind::Audio: return "audio";
    case PortKind::Control: return "control";
    case PortKind::Event: return "event";
    }
    return "unknown";
}

struct Event {
    std::string type;
    float value = 0.0f;
    std::uint64_t timestamp = 0;
};

// Timestamp is a logical frame/sample position, not wall-clock time.
// Queue-level ordering will be added with the stateful scheduler.
inline bool eventComesBefore(const Event& left, const Event& right) {
    return left.timestamp < right.timestamp;
}

template <std::size_t Capacity>
class EventQueue {
public:
    bool push(const Event& event) noexcept {
        if (size_ == Capacity)
            return false;
        std::size_t position = size_;
        while (position > 0 && eventComesBefore(event, events_[position - 1])) {
            events_[position] = events_[position - 1];
            --position;
        }
        events_[position] = event;
        ++size_;
        return true;
    }

    bool empty() const noexcept { return size_ == 0; }
    std::size_t size() const noexcept { return size_; }

    const Event& front() const {
        if (empty())
            throw std::out_of_range("event queue is empty");
        return events_[0];
    }

    bool pop(Event& event) noexcept {
        if (empty())
            return false;
        event = events_[0];
        for (std::size_t index = 1; index < size_; ++index)
            events_[index - 1] = events_[index];
        --size_;
        return true;
    }

    void clear() noexcept { size_ = 0; }

private:
    std::array<Event, Capacity> events_{};
    std::size_t size_ = 0;
};

enum class StreamCategory {
    Audio,
    Control,
    Event,
    Descriptor,
};

inline const char* streamCategoryName(const StreamCategory category) noexcept {
    switch (category) {
    case StreamCategory::Audio: return "audio";
    case StreamCategory::Control: return "control";
    case StreamCategory::Event: return "event";
    case StreamCategory::Descriptor: return "descriptor";
    }
    return "unknown";
}

// Fixed-capacity audio storage. Processing may change samples in place without
// allocating; configure() is intended for setup, not for the realtime thread.
class AudioBlock {
public:
    static constexpr std::size_t maxFrames = 256;
    static constexpr std::size_t maxChannels = 8;

    AudioBlock(const float sampleRate = 48000.0f,
               const std::size_t channels = 2,
               const std::size_t frames = 64) {
        configure(sampleRate, channels, frames);
    }

    void configure(const float sampleRate, const std::size_t channels,
                   const std::size_t frames) {
        if (sampleRate <= 0.0f)
            throw std::invalid_argument("sample rate must be positive");
        if (channels == 0 || channels > maxChannels)
            throw std::out_of_range("audio channel count out of range");
        if (frames == 0 || frames > maxFrames)
            throw std::out_of_range("audio frame count out of range");
        sampleRate_ = sampleRate;
        channels_ = channels;
        frames_ = frames;
    }

    float sampleRate() const noexcept { return sampleRate_; }
    std::size_t channels() const noexcept { return channels_; }
    std::size_t frames() const noexcept { return frames_; }

    bool matches(const AudioBlock& other) const noexcept {
        return sampleRate_ == other.sampleRate_ && channels_ == other.channels_
            && frames_ == other.frames_;
    }

    float& at(const std::size_t channel, const std::size_t frame) {
        check(channel, frame);
        return samples_[channel * maxFrames + frame];
    }

    const float& at(const std::size_t channel, const std::size_t frame) const {
        check(channel, frame);
        return samples_[channel * maxFrames + frame];
    }

    void copyFrom(const AudioBlock& source) noexcept {
        if (!matches(source))
            return;
        for (std::size_t channel = 0; channel < channels_; ++channel)
            std::copy_n(source.samples_.data() + channel * maxFrames, frames_,
                        samples_.data() + channel * maxFrames);
    }

    void clear() noexcept {
        for (std::size_t channel = 0; channel < channels_; ++channel)
            std::fill_n(samples_.data() + channel * maxFrames, frames_, 0.0f);
    }

private:
    void check(const std::size_t channel, const std::size_t frame) const {
        if (channel >= channels_ || frame >= frames_)
            throw std::out_of_range("audio sample out of range");
    }

    float sampleRate_ = 48000.0f;
    std::size_t channels_ = 2;
    std::size_t frames_ = 64;
    std::array<float, maxChannels * maxFrames> samples_{};
};

// Stream metadata is negotiated during preparation and must not be mutated by
// the realtime processing path. It generalizes AudioBlock to descriptors,
// gestures and other time-tagged multidimensional streams.
struct StreamDescriptor {
    bool timeTagged = false;
    double rate = 0.0;
    double offset = 0.0;
    std::size_t width = 0;
    std::size_t height = 0;
    std::size_t maxFrames = 0;
    std::string domain;
    std::string unit;
    std::vector<std::string> labels;
    StreamCategory category = StreamCategory::Audio;

    bool compatibleWith(const StreamDescriptor& other) const noexcept {
        return timeTagged == other.timeTagged && rate == other.rate
            && width == other.width && height == other.height
            && maxFrames == other.maxFrames && domain == other.domain
            && unit == other.unit && labels == other.labels
            && category == other.category;
    }
};

class ControlBlock {
public:
    static constexpr std::size_t maxValues = 256;
    explicit ControlBlock(const std::size_t values = 1) { configure(values); }
    void configure(const std::size_t values) {
        if (values == 0 || values > maxValues)
            throw std::out_of_range("control value count out of range");
        values_ = values;
    }
    std::size_t size() const noexcept { return values_; }
    float& at(const std::size_t index) {
        if (index >= values_)
            throw std::out_of_range("control value out of range");
        return valuesData_[index];
    }
    const float& at(const std::size_t index) const {
        if (index >= values_)
            throw std::out_of_range("control value out of range");
        return valuesData_[index];
    }
    void clear() noexcept { std::fill_n(valuesData_.data(), values_, 0.0f); }

private:
    std::array<float, maxValues> valuesData_{};
    std::size_t values_ = 1;
};

class DescriptorBlock {
public:
    static constexpr std::size_t maxWidth = 64;
    static constexpr std::size_t maxFrames = 256;
    DescriptorBlock(const std::size_t width = 1, const std::size_t frames = 1) {
        configure(width, frames);
    }
    void configure(const std::size_t width, const std::size_t frames) {
        if (width == 0 || width > maxWidth || frames == 0 || frames > maxFrames)
            throw std::out_of_range("descriptor block dimensions out of range");
        width_ = width;
        frames_ = frames;
    }
    std::size_t width() const noexcept { return width_; }
    std::size_t frames() const noexcept { return frames_; }
    float& at(const std::size_t width, const std::size_t frame) {
        check(width, frame);
        return values_[frame * maxWidth + width];
    }
    const float& at(const std::size_t width, const std::size_t frame) const {
        check(width, frame);
        return values_[frame * maxWidth + width];
    }
    void clear() noexcept { std::fill_n(values_.data(), width_ * frames_, 0.0f); }

private:
    void check(const std::size_t width, const std::size_t frame) const {
        if (width >= width_ || frame >= frames_)
            throw std::out_of_range("descriptor value out of range");
    }
    std::array<float, maxWidth * maxFrames> values_{};
    std::size_t width_ = 1;
    std::size_t frames_ = 1;
};

class EventBlock {
public:
    bool push(const Event& event) noexcept { return queue_.push(event); }
    bool pop(Event& event) noexcept { return queue_.pop(event); }
    bool empty() const noexcept { return queue_.empty(); }
    std::size_t size() const noexcept { return queue_.size(); }
    void clear() noexcept { queue_.clear(); }

private:
    EventQueue<64> queue_;
};

enum class PortDirection {
    Input,
    Output,
};

struct MultimodalPort {
    std::string name;
    PortDirection direction = PortDirection::Input;
    StreamCategory category = StreamCategory::Control;
    StreamDescriptor stream;

    bool compatibleWith(const MultimodalPort& other) const noexcept {
        return direction != other.direction && category == other.category
            && stream.compatibleWith(other.stream);
    }
};

using MultimodalPayload = std::variant<AudioBlock, ControlBlock, DescriptorBlock, EventBlock>;

inline StreamCategory payloadCategory(const MultimodalPayload& payload) noexcept {
    return std::visit([](const auto& value) {
        using Value = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<Value, AudioBlock>)
            return StreamCategory::Audio;
        if constexpr (std::is_same_v<Value, ControlBlock>)
            return StreamCategory::Control;
        if constexpr (std::is_same_v<Value, DescriptorBlock>)
            return StreamCategory::Descriptor;
        return StreamCategory::Event;
    }, payload);
}

inline std::string escapeJson(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size() + 8);
    for (const char character : value) {
        switch (character) {
        case '\\': escaped += "\\\\"; break;
        case '"': escaped += "\\\""; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default: escaped += character; break;
        }
    }
    return escaped;
}

struct PayloadProvenance {
    std::string source;
    std::string transform;
    std::string version;
    std::string license;
};

// Authorial RASGO envelope for metadata exchanged between instruments. The
// envelope is prepared and serialized outside the realtime callback; the
// payload itself remains a fixed-capacity MultimodalPayload.
struct PayloadEnvelope {
    std::uint32_t schemaVersion = 1;
    std::string origin;
    std::uint64_t frameTimestamp = 0;
    double logicalTime = 0.0;
    StreamCategory category = StreamCategory::Audio;
    MultimodalPayload payload = AudioBlock{};
    std::optional<StreamDescriptor> stream;
    bool latencyKnown = false;
    double latencySeconds = 0.0;
    PayloadProvenance provenance;

    bool valid() const noexcept {
        if (schemaVersion == 0 || origin.empty() || logicalTime < 0.0
            || (latencyKnown && latencySeconds < 0.0)
            || payloadCategory(payload) != category)
            return false;
        if (stream.has_value() && (stream->category != category
                                   || stream->width == 0
                                   || stream->height == 0
                                   || stream->maxFrames == 0))
            return false;
        return !provenance.source.empty() && !provenance.version.empty();
    }

    // Metadata-only representation for manifests and archival records. It is
    // intentionally not used by process/processAt().
    std::string toJson() const {
        std::ostringstream json;
        json << "{\"schema_version\":" << schemaVersion
             << ",\"origin\":\"" << escapeJson(origin)
             << "\",\"frame_timestamp\":" << frameTimestamp
             << ",\"logical_time\":" << logicalTime
             << ",\"category\":\"" << streamCategoryName(category)
             << "\",\"latency_known\":" << (latencyKnown ? "true" : "false")
             << ",\"latency_seconds\":" << latencySeconds
             << ",\"provenance_source\":\"" << escapeJson(provenance.source)
             << "\",\"provenance_transform\":\"" << escapeJson(provenance.transform)
             << "\",\"provenance_version\":\"" << escapeJson(provenance.version)
             << "\",\"provenance_license\":\"" << escapeJson(provenance.license)
             << "\"}";
        return json.str();
    }
};

// Archival serialization is a preparation-time operation. It validates the
// envelope and emits a versioned metadata record; payload bytes remain the
// responsibility of a storage adapter so realtime code never performs I/O.
class ArchiveSerializer {
public:
    static constexpr std::uint32_t formatVersion = 1;

    static bool serializeEnvelope(const PayloadEnvelope& envelope,
                                  std::string& output) {
        if (!envelope.valid()) {
            output.clear();
            return false;
        }
        std::ostringstream archive;
        archive << "{\"archive_format\":\"rasgo-payload-envelope\""
                << ",\"format_version\":" << formatVersion
                << ",\"payload_encoding\":\"fixed-block-external\""
                << ",\"envelope\":" << envelope.toJson() << "}";
        output = archive.str();
        return true;
    }
};

struct MultimodalConnection {
    std::size_t source = 0;
    std::size_t target = 0;
};

class MultimodalGraph {
public:
    std::size_t addPort(MultimodalPort port) {
        ports_.push_back(std::move(port));
        prepared_ = false;
        return ports_.size() - 1;
    }

    std::size_t portCount() const noexcept { return ports_.size(); }
    std::size_t connectionCount() const noexcept { return connections_.size(); }
    const MultimodalPort& port(const std::size_t id) const {
        if (id >= ports_.size())
            throw std::out_of_range("multimodal port out of range");
        return ports_[id];
    }

    void connect(const std::size_t source, const std::size_t target) {
        const auto& sourcePort = port(source);
        const auto& targetPort = port(target);
        if (sourcePort.direction != PortDirection::Output
            || targetPort.direction != PortDirection::Input)
            throw std::invalid_argument("multimodal connection direction mismatch");
        if (!sourcePort.compatibleWith(targetPort))
            throw std::invalid_argument("multimodal port contract mismatch");
        for (const auto& connection : connections_)
            if (connection.target == target)
                throw std::invalid_argument("multimodal fan-in requires an explicit mixer");
        connections_.push_back({source, target});
        prepared_ = false;
    }

    void prepare() {
        evaluationOrder_ = evaluationOrder();
        prepared_ = true;
    }

    bool prepared() const noexcept { return prepared_; }
    const std::vector<std::size_t>& evaluation() const noexcept { return evaluationOrder_; }

    std::string manifest() const {
        std::ostringstream stream;
        stream << "{\"version\":1,\"ports\":[";
        for (std::size_t index = 0; index < ports_.size(); ++index) {
            if (index != 0)
                stream << ',';
            stream << "{\"name\":\"" << ports_[index].name
                   << "\",\"direction\":\""
                   << (ports_[index].direction == PortDirection::Input ? "input" : "output")
                   << "\"}";
        }
        stream << "]}";
        return stream.str();
    }

private:
    std::vector<std::size_t> evaluationOrder() const {
        std::vector<std::vector<std::size_t>> outgoing(ports_.size());
        for (const auto& connection : connections_)
            outgoing[connection.source].push_back(connection.target);
        std::vector<int> marks(ports_.size(), 0);
        std::vector<std::size_t> order;
        std::function<void(std::size_t)> visit = [&](const std::size_t id) {
            if (marks[id] == 1)
                throw std::logic_error("multimodal graph cycle requires scheduler");
            if (marks[id] == 2)
                return;
            marks[id] = 1;
            for (const auto target : outgoing[id])
                visit(target);
            marks[id] = 2;
            order.push_back(id);
        };
        for (std::size_t id = 0; id < ports_.size(); ++id)
            visit(id);
        std::reverse(order.begin(), order.end());
        return order;
    }

    std::vector<MultimodalPort> ports_;
    std::vector<MultimodalConnection> connections_;
    std::vector<std::size_t> evaluationOrder_;
    bool prepared_ = false;
};

struct PortDescriptor {
    PortDescriptor() = default;
    PortDescriptor(std::string portName, const PortKind portKind, std::string portUnit,
                   std::optional<StreamDescriptor> streamDescriptor = std::nullopt)
        : name(std::move(portName)), kind(portKind), unit(std::move(portUnit)),
          stream(std::move(streamDescriptor)) {}

    std::string name;
    PortKind kind = PortKind::Control;
    std::string unit;
    std::optional<StreamDescriptor> stream;
};

class AudioProcessor {
public:
    virtual ~AudioProcessor() = default;

    // Returns false when the preconfigured blocks are incompatible. The
    // realtime path does not allocate and does not throw for this condition.
    virtual bool processAudio(const AudioBlock& input, AudioBlock& output) noexcept = 0;
};

class AudioGainProcessor final : public AudioProcessor {
public:
    explicit AudioGainProcessor(const float gain = 1.0f) noexcept : gain_(gain) {}

    void setGain(const float gain) noexcept { gain_ = gain; }
    float gain() const noexcept { return gain_; }

    bool processAudio(const AudioBlock& input, AudioBlock& output) noexcept override {
        if (input.channels() != output.channels() || input.frames() != output.frames()
            || input.sampleRate() != output.sampleRate())
            return false;

        for (std::size_t channel = 0; channel < input.channels(); ++channel)
            for (std::size_t frame = 0; frame < input.frames(); ++frame)
                output.at(channel, frame) = input.at(channel, frame) * gain_;
        return true;
    }

private:
    float gain_ = 1.0f;
};

class MultimodalProcessor {
public:
    virtual ~MultimodalProcessor() = default;
    virtual StreamCategory category() const noexcept = 0;
    virtual bool process(const MultimodalPayload& input,
                         MultimodalPayload& output) noexcept = 0;
    virtual void reset() noexcept {}
};

struct ProcessContext {
    std::uint64_t frameStart = 0;
    double time = 0.0;
    std::size_t frames = 0;
};

struct MultimodalSnapshot {
    bool prepared = false;
    StreamCategory category = StreamCategory::Audio;
    ProcessContext context;
};

class AudioPayloadProcessor final : public MultimodalProcessor {
public:
    explicit AudioPayloadProcessor(const float gain = 1.0f) noexcept : processor_(gain) {}
    StreamCategory category() const noexcept override { return StreamCategory::Audio; }
    bool process(const MultimodalPayload& input, MultimodalPayload& output) noexcept override {
        if (!std::holds_alternative<AudioBlock>(input)
            || !std::holds_alternative<AudioBlock>(output))
            return false;
        return processor_.processAudio(std::get<AudioBlock>(input),
                                       std::get<AudioBlock>(output));
    }

private:
    AudioGainProcessor processor_;
};

class ControlPayloadProcessor final : public MultimodalProcessor {
public:
    StreamCategory category() const noexcept override { return StreamCategory::Control; }
    bool process(const MultimodalPayload& input, MultimodalPayload& output) noexcept override {
        if (!std::holds_alternative<ControlBlock>(input)
            || !std::holds_alternative<ControlBlock>(output))
            return false;
        const auto& source = std::get<ControlBlock>(input);
        auto& target = std::get<ControlBlock>(output);
        if (source.size() != target.size())
            return false;
        for (std::size_t index = 0; index < source.size(); ++index)
            target.at(index) = source.at(index);
        return true;
    }
};

class DescriptorPayloadProcessor final : public MultimodalProcessor {
public:
    StreamCategory category() const noexcept override { return StreamCategory::Descriptor; }
    bool process(const MultimodalPayload& input, MultimodalPayload& output) noexcept override {
        if (!std::holds_alternative<DescriptorBlock>(input)
            || !std::holds_alternative<DescriptorBlock>(output))
            return false;
        const auto& source = std::get<DescriptorBlock>(input);
        auto& target = std::get<DescriptorBlock>(output);
        if (source.width() != target.width() || source.frames() != target.frames())
            return false;
        for (std::size_t frame = 0; frame < source.frames(); ++frame)
            for (std::size_t width = 0; width < source.width(); ++width)
                target.at(width, frame) = source.at(width, frame);
        return true;
    }
};

class EventPayloadProcessor final : public MultimodalProcessor {
public:
    StreamCategory category() const noexcept override { return StreamCategory::Event; }
    bool process(const MultimodalPayload& input, MultimodalPayload& output) noexcept override {
        if (!std::holds_alternative<EventBlock>(input)
            || !std::holds_alternative<EventBlock>(output))
            return false;
        auto source = std::get<EventBlock>(input);
        auto& target = std::get<EventBlock>(output);
        target.clear();
        Event event;
        while (source.pop(event) && target.push(event)) {}
        return source.empty();
    }
};

class MultimodalChain {
public:
    std::size_t add(std::unique_ptr<MultimodalProcessor> processor) {
        if (!processor)
            throw std::invalid_argument("cannot add null multimodal processor");
        processors_.push_back(std::move(processor));
        prepared_ = false;
        return processors_.size() - 1;
    }

    void prepare(const StreamCategory category) {
        for (const auto& processor : processors_)
            if (processor->category() != category)
                throw std::invalid_argument("multimodal processor category mismatch");
        category_ = category;
        prepared_ = true;
    }

    bool process(const MultimodalPayload& input, MultimodalPayload& output) noexcept {
        return processAt(input, output, {});
    }

    bool processAt(const MultimodalPayload& input, MultimodalPayload& output,
                   const ProcessContext context) noexcept {
        if (!prepared_ || payloadCategory(input) != category_
            || payloadCategory(output) != category_)
            return false;
        context_ = context;
        configureStaging(input);
        const MultimodalPayload* current = &input;
        MultimodalPayload* next = &staging_[0];
        for (const auto& processor : processors_) {
            if (!processor->process(*current, *next))
                return false;
            current = next;
            next = next == &staging_[0] ? &staging_[1] : &staging_[0];
        }
        output = *current;
        return true;
    }

    void reset() noexcept {
        for (const auto& processor : processors_)
            processor->reset();
        context_ = {};
    }

    MultimodalSnapshot snapshot() const noexcept {
        return {prepared_, category_, context_};
    }

    std::size_t processorCount() const noexcept { return processors_.size(); }
    bool realtimeSafeBase() const noexcept { return true; }

private:
    void configureStaging(const MultimodalPayload& input) noexcept {
        for (auto& staging : staging_) {
            std::visit([&staging](const auto& value) {
                using Value = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<Value, AudioBlock>) {
                    staging = value;
                    std::get<AudioBlock>(staging).configure(value.sampleRate(),
                                                            value.channels(), value.frames());
                } else if constexpr (std::is_same_v<Value, ControlBlock>) {
                    staging = value;
                    std::get<ControlBlock>(staging).configure(value.size());
                } else if constexpr (std::is_same_v<Value, DescriptorBlock>) {
                    staging = value;
                    std::get<DescriptorBlock>(staging).configure(value.width(), value.frames());
                } else {
                    staging = EventBlock{};
                    std::get<EventBlock>(staging).clear();
                }
            }, input);
        }
    }

    std::vector<std::unique_ptr<MultimodalProcessor>> processors_;
    std::array<MultimodalPayload, 2> staging_{};
    StreamCategory category_ = StreamCategory::Audio;
    ProcessContext context_;
    bool prepared_ = false;
};

struct AudioConnection {
    std::size_t source = 0;
    std::size_t target = 0;
};

// Block graph. Nodes and buffers are prepared before process(); the execution
// path only traverses precomputed storage and performs no dynamic allocation.
class AudioGraph {
public:
    std::size_t add(std::unique_ptr<AudioProcessor> processor) {
        if (!processor)
            throw std::invalid_argument("cannot add a null audio processor");
        nodes_.push_back(std::move(processor));
        prepared_ = false;
        return nodes_.size() - 1;
    }

    void connect(const std::size_t source, const std::size_t target) {
        checkNode(source);
        checkNode(target);
        for (const auto& connection : connections_)
            if (connection.target == target)
                throw std::invalid_argument("audio node already has an input");
        connections_.push_back({source, target});
        prepared_ = false;
    }

    void prepare(const float sampleRate, const std::size_t channels,
                 const std::size_t frames) {
        prepare(StreamDescriptor{true, sampleRate, 0.0, channels, 1, frames,
                                 "samples", "linear", {}});
    }

    void prepare(const StreamDescriptor& descriptor) {
        if (descriptor.category != StreamCategory::Audio || descriptor.width == 0
            || descriptor.height != 1
            || descriptor.maxFrames == 0 || descriptor.rate <= 0.0
            || (!descriptor.labels.empty() && descriptor.labels.size() != descriptor.width))
            throw std::invalid_argument("invalid audio stream descriptor");
        AudioBlock configuration(static_cast<float>(descriptor.rate),
                                 descriptor.width, descriptor.maxFrames);
        nodeBlocks_.assign(nodes_.size(), configuration);
        evaluationOrder_ = evaluationOrder();
        descriptor_ = descriptor;
        prepared_ = true;
    }

    const StreamDescriptor& descriptor() const noexcept { return descriptor_; }

    bool process(const AudioBlock& input, AudioBlock& output) noexcept {
        if (!prepared_ || !input.matches(output) || nodeBlocks_.empty())
            return false;

        for (const auto nodeId : evaluationOrder_) {
            const AudioBlock* source = &input;
            for (const auto& connection : connections_)
                if (connection.target == nodeId)
                    source = &nodeBlocks_[connection.source];
            if (!nodes_[nodeId]->processAudio(*source, nodeBlocks_[nodeId]))
                return false;
        }
        output.copyFrom(nodeBlocks_[evaluationOrder_.back()]);
        return true;
    }

private:
    void checkNode(const std::size_t id) const {
        if (id >= nodes_.size())
            throw std::out_of_range("audio node out of range");
    }

    std::vector<std::size_t> evaluationOrder() const {
        std::vector<std::vector<std::size_t>> outgoing(nodes_.size());
        for (const auto& connection : connections_)
            outgoing[connection.source].push_back(connection.target);

        std::vector<int> marks(nodes_.size(), 0);
        std::vector<std::size_t> order;
        std::function<void(std::size_t)> visit = [&](const std::size_t id) {
            if (marks[id] == 1)
                throw std::logic_error("audio graph cycle is not supported");
            if (marks[id] == 2)
                return;
            marks[id] = 1;
            for (const auto target : outgoing[id])
                visit(target);
            marks[id] = 2;
            order.push_back(id);
        };
        for (std::size_t id = 0; id < nodes_.size(); ++id)
            visit(id);
        std::reverse(order.begin(), order.end());
        return order;
    }

    std::vector<std::unique_ptr<AudioProcessor>> nodes_;
    std::vector<AudioConnection> connections_;
    std::vector<AudioBlock> nodeBlocks_;
    std::vector<std::size_t> evaluationOrder_;
    StreamDescriptor descriptor_;
    bool prepared_ = false;
};

using PortValue = std::variant<float, Event>;

struct ParameterDescriptor {
    std::string id;
    float minimum = 0.0f;
    float maximum = 1.0f;
    float defaultValue = 0.0f;
    std::string unit;
};

struct Parameter {
    ParameterDescriptor descriptor;
    float value = 0.0f;
};

class Module {
public:
    virtual ~Module() = default;

    virtual std::string type() const = 0;
    virtual void process() = 0;
    virtual std::string state() const = 0;
    virtual void restoreState(const std::string& serialized) = 0;

    PortKind inputKind(const std::size_t port) const {
        if (port >= inputPorts_.size())
            throw std::out_of_range("input port out of range");
        return inputPorts_[port].kind;
    }

    PortKind outputKind(const std::size_t port) const {
        if (port >= outputPorts_.size())
            throw std::out_of_range("output port out of range");
        return outputPorts_[port].kind;
    }

    const PortDescriptor& inputDescriptor(const std::size_t port) const {
        if (port >= inputPorts_.size())
            throw std::out_of_range("input port out of range");
        return inputPorts_[port];
    }

    const PortDescriptor& outputDescriptor(const std::size_t port) const {
        if (port >= outputPorts_.size())
            throw std::out_of_range("output port out of range");
        return outputPorts_[port];
    }

    void setInputStreamDescriptor(const std::size_t port,
                                  const StreamDescriptor& descriptor) {
        if (port >= inputPorts_.size())
            throw std::out_of_range("input port out of range");
        inputPorts_[port].stream = descriptor;
    }

    void setOutputStreamDescriptor(const std::size_t port,
                                   const StreamDescriptor& descriptor) {
        if (port >= outputPorts_.size())
            throw std::out_of_range("output port out of range");
        outputPorts_[port].stream = descriptor;
    }

    const std::vector<Parameter>& parameters() const { return parameters_; }

    std::string manifest() const {
        std::ostringstream stream;
        stream << "{\"version\":1,\"type\":\"" << type() << "\",\"inputs\":[";
        for (std::size_t index = 0; index < inputPorts_.size(); ++index) {
            if (index != 0)
                stream << ',';
            stream << "{\"name\":\"" << inputPorts_[index].name
                   << "\",\"kind\":\"" << portKindName(inputPorts_[index].kind)
                   << "\",\"unit\":\"" << inputPorts_[index].unit << "\"}";
        }
        stream << "],\"outputs\":[";
        for (std::size_t index = 0; index < outputPorts_.size(); ++index) {
            if (index != 0)
                stream << ',';
            stream << "{\"name\":\"" << outputPorts_[index].name
                   << "\",\"kind\":\"" << portKindName(outputPorts_[index].kind)
                   << "\",\"unit\":\"" << outputPorts_[index].unit << "\"}";
        }
        stream << "],\"parameters\":[";
        for (std::size_t index = 0; index < parameters_.size(); ++index) {
            if (index != 0)
                stream << ',';
            const auto& parameter = parameters_[index];
            stream << "{\"id\":\"" << parameter.descriptor.id
                   << "\",\"min\":" << parameter.descriptor.minimum
                   << ",\"max\":" << parameter.descriptor.maximum
                   << ",\"default\":" << parameter.descriptor.defaultValue
                   << ",\"unit\":\"" << parameter.descriptor.unit << "\"}";
        }
        stream << "]}";
        return stream.str();
    }

    std::string parameterState() const {
        std::ostringstream stream;
        bool first = true;
        for (const auto& parameter : parameters_) {
            if (!first)
                stream << ';';
            first = false;
            stream << parameter.descriptor.id << '=' << std::setprecision(9)
                   << parameter.value;
        }
        return stream.str();
    }

    void restoreParameterState(const std::string& serialized) {
        std::istringstream entries(serialized);
        std::string entry;
        while (std::getline(entries, entry, ';')) {
            if (entry.empty())
                continue;
            const auto separator = entry.find('=');
            if (separator == std::string::npos)
                throw std::invalid_argument("invalid parameter state");
            const auto id = entry.substr(0, separator);
            std::size_t consumed = 0;
            const auto value = std::stof(entry.substr(separator + 1), &consumed);
            if (consumed != entry.size() - separator - 1)
                throw std::invalid_argument("invalid parameter value");
            setParameter(id, value);
        }
    }

    void setParameter(const std::string& id, const float value) {
        for (auto& parameter : parameters_) {
            if (parameter.descriptor.id == id) {
                if (value < parameter.descriptor.minimum || value > parameter.descriptor.maximum)
                    throw std::out_of_range("parameter value out of range");
                parameter.value = value;
                return;
            }
        }
        throw std::invalid_argument("unknown parameter: " + id);
    }

    void setInput(const std::size_t port, const float value) {
        if (port >= inputs_.size())
            throw std::out_of_range("input port out of range");
        inputs_[port] = value;
    }

    void setInput(const std::size_t port, const Event& event) {
        if (port >= inputs_.size())
            throw std::out_of_range("input port out of range");
        inputs_[port] = event;
    }

    void setInputValue(const std::size_t port, const PortValue& value) {
        if (port >= inputs_.size())
            throw std::out_of_range("input port out of range");
        inputs_[port] = value;
    }

    float output(const std::size_t port) const {
        if (port >= outputs_.size())
            throw std::out_of_range("output port out of range");
        return std::get<float>(outputs_[port]);
    }

    const Event& eventOutput(const std::size_t port) const {
        if (port >= outputs_.size())
            throw std::out_of_range("output port out of range");
        return std::get<Event>(outputs_[port]);
    }

    PortValue outputValue(const std::size_t port) const {
        if (port >= outputs_.size())
            throw std::out_of_range("output port out of range");
        return outputs_[port];
    }

protected:
    explicit Module(std::vector<PortDescriptor> inputPorts,
                    std::vector<PortDescriptor> outputPorts,
                    std::vector<ParameterDescriptor> parameterDescriptors = {})
        : inputs_(inputPorts.size(), 0.0f), outputs_(outputPorts.size(), 0.0f),
          inputPorts_(std::move(inputPorts)), outputPorts_(std::move(outputPorts)) {
        for (auto& descriptor : parameterDescriptors)
            parameters_.push_back({descriptor, descriptor.defaultValue});
    }

    float parameterValue(const std::string& id) const {
        for (const auto& parameter : parameters_)
            if (parameter.descriptor.id == id)
                return parameter.value;
        throw std::invalid_argument("unknown parameter: " + id);
    }

    float input(const std::size_t port) const {
        if (port >= inputs_.size())
            throw std::out_of_range("input port out of range");
        return std::get<float>(inputs_[port]);
    }

    const Event& eventInput(const std::size_t port) const {
        if (port >= inputs_.size())
            throw std::out_of_range("input port out of range");
        return std::get<Event>(inputs_[port]);
    }

    void setOutput(const std::size_t port, const float value) {
        if (port >= outputs_.size())
            throw std::out_of_range("output port out of range");
        outputs_[port] = value;
    }

    void setOutput(const std::size_t port, const Event& event) {
        if (port >= outputs_.size())
            throw std::out_of_range("output port out of range");
        outputs_[port] = event;
    }

private:
    std::vector<PortValue> inputs_;
    std::vector<PortValue> outputs_;
    std::vector<PortDescriptor> inputPorts_;
    std::vector<PortDescriptor> outputPorts_;
    std::vector<Parameter> parameters_;
};

class ConstantSource final : public Module {
public:
    explicit ConstantSource(const float value = 0.0f)
        : Module({}, {{"out", PortKind::Audio, "signal"}}), value_(value) {}

    std::string type() const override { return "SOURCE.CONSTANT"; }

    void process() override { setOutput(0, value_); }

    std::string state() const override {
        std::ostringstream stream;
        stream << std::setprecision(9) << value_;
        return stream.str();
    }

    void restoreState(const std::string& serialized) override {
        value_ = parseFloat(serialized);
    }

private:
    static float parseFloat(const std::string& text) {
        std::size_t consumed = 0;
        const float value = std::stof(text, &consumed);
        if (consumed != text.size())
            throw std::invalid_argument("invalid constant state");
        return value;
    }

    float value_;
};

class Gain final : public Module {
public:
    explicit Gain(const float gain = 1.0f)
        : Module({{"in", PortKind::Audio, "signal"}},
                 {{"out", PortKind::Audio, "signal"}},
                 {{"gain", 0.0f, 16.0f, 1.0f, "linear"}}) {
        setParameter("gain", gain);
    }

    std::string type() const override { return "TRANSFORM.GAIN"; }

    void process() override { setOutput(0, input(0) * parameterValue("gain")); }

    std::string state() const override {
        return {};
    }

    void restoreState(const std::string&) override {}
};

class Meter final : public Module {
public:
    Meter() : Module({{"in", PortKind::Audio, "signal"}},
                     {{"out", PortKind::Audio, "signal"}}) {}

    std::string type() const override { return "METER.VALUE"; }

    void process() override {
        observed_ = input(0);
        setOutput(0, observed_);
    }

    std::string state() const override {
        std::ostringstream stream;
        stream << std::setprecision(9) << observed_;
        return stream.str();
    }

    void restoreState(const std::string& serialized) override {
        std::size_t consumed = 0;
        observed_ = std::stof(serialized, &consumed);
        if (consumed != serialized.size())
            throw std::invalid_argument("invalid meter state");
    }

private:
    float observed_ = 0.0f;
};

class ControlSource final : public Module {
public:
    explicit ControlSource(const float value = 0.0f)
        : Module({}, {{"out", PortKind::Control, "normalized"}}), value_(value) {}

    std::string type() const override { return "INPUT.CONTROL"; }

    void process() override { setOutput(0, value_); }

    std::string state() const override {
        std::ostringstream stream;
        stream << std::setprecision(9) << value_;
        return stream.str();
    }

    void restoreState(const std::string& serialized) override {
        std::size_t consumed = 0;
        value_ = std::stof(serialized, &consumed);
        if (consumed != serialized.size())
            throw std::invalid_argument("invalid control state");
    }

private:
    float value_;
};

class EventSource final : public Module {
public:
    explicit EventSource(Event event = {})
        : Module({}, {{"out", PortKind::Event, "event"}}), event_(std::move(event)) {}

    std::string type() const override { return "EVENT.SOURCE"; }

    void process() override { setOutput(0, event_); }

    std::string state() const override {
        std::ostringstream stream;
        stream << std::quoted(event_.type) << ' ' << std::setprecision(9)
               << event_.value << ' ' << event_.timestamp;
        return stream.str();
    }

    void restoreState(const std::string& serialized) override {
        std::istringstream stream(serialized);
        if (!(stream >> std::quoted(event_.type) >> event_.value >> event_.timestamp))
            throw std::invalid_argument("invalid event state");
    }

private:
    Event event_;
};

class EventGate final : public Module {
public:
    EventGate() : Module({{"in", PortKind::Event, "event"}},
                         {{"out", PortKind::Control, "gate"}}) {}

    std::string type() const override { return "EVENT.GATE"; }

    void process() override {
        const auto& event = eventInput(0);
        setOutput(0, event.type.empty() ? 0.0f : 1.0f);
    }

    std::string state() const override { return "0"; }
    void restoreState(const std::string&) override {}
};

class Clock final : public Module {
public:
    explicit Clock(const std::uint64_t timestamp = 0)
        : Module({}, { {"out", PortKind::Event, "tick"} },
                 { {"interval", 1.0f, 1000000.0f, 1.0f, "frames"} }),
          timestamp_(timestamp) {}

    std::string type() const override { return "TIME.CLOCK"; }

    void process() override {
        setOutput(0, Event{"tick", 1.0f, timestamp_});
        timestamp_ += static_cast<std::uint64_t>(parameterValue("interval"));
    }

    std::string state() const override { return std::to_string(timestamp_); }

    void restoreState(const std::string& serialized) override {
        std::size_t consumed = 0;
        timestamp_ = std::stoull(serialized, &consumed);
        if (consumed != serialized.size())
            throw std::invalid_argument("invalid clock state");
    }

private:
    std::uint64_t timestamp_ = 0;
};

class Probability final : public Module {
public:
    explicit Probability(const std::uint32_t seed = 1)
        : Module({ {"in", PortKind::Event, "event"} },
                 { {"out", PortKind::Event, "event"} },
                 { {"probability", 0.0f, 1.0f, 0.5f, "ratio"} }),
          seed_(seed == 0 ? 1 : seed) {}

    std::string type() const override { return "DECISION.PROBABILITY"; }

    void process() override {
        const auto inputEvent = eventInput(0);
        if (inputEvent.type.empty()) {
            setOutput(0, Event{});
            return;
        }
        seed_ ^= seed_ << 13;
        seed_ ^= seed_ >> 17;
        seed_ ^= seed_ << 5;
        const auto roll = static_cast<float>(seed_ % 1000000u) / 1000000.0f;
        setOutput(0, roll < parameterValue("probability") ? inputEvent : Event{});
    }

    std::string state() const override { return std::to_string(seed_); }

    void restoreState(const std::string& serialized) override {
        std::size_t consumed = 0;
        seed_ = static_cast<std::uint32_t>(std::stoul(serialized, &consumed));
        if (consumed != serialized.size() || seed_ == 0)
            throw std::invalid_argument("invalid probability state");
    }

private:
    std::uint32_t seed_ = 1;
};

class Split final : public Module {
public:
    Split() : Module({ {"in", PortKind::Audio, "signal"} },
                     { {"a", PortKind::Audio, "signal"},
                       {"b", PortKind::Audio, "signal"} }) {}

    std::string type() const override { return "ROUTE.SPLIT"; }
    void process() override {
        setOutput(0, input(0));
        setOutput(1, input(0));
    }
    std::string state() const override { return {}; }
    void restoreState(const std::string&) override {}
};

class Sum final : public Module {
public:
    Sum() : Module({ {"a", PortKind::Audio, "signal"},
                     {"b", PortKind::Audio, "signal"} },
                    { {"out", PortKind::Audio, "signal"} }) {}

    std::string type() const override { return "MIX.SUM"; }
    void process() override { setOutput(0, input(0) + input(1)); }
    std::string state() const override { return {}; }
    void restoreState(const std::string&) override {}
};

struct Connection {
    std::size_t source = 0;
    std::size_t sourcePort = 0;
    std::size_t target = 0;
    std::size_t targetPort = 0;
};

class Graph {
public:
    using Factory = std::function<std::unique_ptr<Module>(const std::string&)>;

    std::size_t add(std::unique_ptr<Module> module) {
        if (!module)
            throw std::invalid_argument("cannot add a null module");
        nodes_.push_back(std::move(module));
        return nodes_.size() - 1;
    }

    std::size_t nodeCount() const { return nodes_.size(); }

    const PortDescriptor& nodeOutputDescriptor(const std::size_t nodeId,
                                                const std::size_t port) const {
        return node(nodeId).outputDescriptor(port);
    }

    std::string nodeManifest(const std::size_t nodeId) const {
        return node(nodeId).manifest();
    }

    void connect(const std::size_t source, const std::size_t sourcePort,
                 const std::size_t target, const std::size_t targetPort) {
        node(source);
        node(target);
        if (node(source).outputKind(sourcePort) != node(target).inputKind(targetPort))
            throw std::invalid_argument("incompatible graph port kinds");
        const auto& output = node(source).outputDescriptor(sourcePort);
        const auto& input = node(target).inputDescriptor(targetPort);
        if (output.stream && input.stream && !output.stream->compatibleWith(*input.stream))
            throw std::invalid_argument("incompatible graph stream descriptors");
        connections_.push_back({source, sourcePort, target, targetPort});
    }

    void process() {
        for (const auto nodeId : evaluationOrder()) {
            node(nodeId).process();
            for (const auto& connection : connections_) {
                if (connection.source == nodeId)
                    node(connection.target).setInputValue(connection.targetPort,
                                                          node(nodeId).outputValue(connection.sourcePort));
            }
        }
    }

    float output(const std::size_t nodeId, const std::size_t port) const {
        return node(nodeId).output(port);
    }

    const Event& eventOutput(const std::size_t nodeId, const std::size_t port) const {
        return node(nodeId).eventOutput(port);
    }

    void setParameter(const std::size_t nodeId, const std::string& id, const float value) {
        node(nodeId).setParameter(id, value);
    }

    std::string serialize() const {
        std::ostringstream stream;
        stream << "RASGO_PATCH 2\n";
        stream << "NODES " << nodes_.size() << "\n";
        for (std::size_t id = 0; id < nodes_.size(); ++id)
            stream << "NODE " << id << ' ' << nodes_[id]->type() << ' '
                   << std::quoted(nodes_[id]->state()) << ' '
                   << std::quoted(nodes_[id]->parameterState()) << "\n";
        stream << "CONNECTIONS " << connections_.size() << "\n";
        for (const auto& connection : connections_)
            stream << "CONNECT " << connection.source << ' ' << connection.sourcePort << ' '
                   << connection.target << ' ' << connection.targetPort << "\n";
        return stream.str();
    }

    static Graph deserialize(const std::string& serialized, const Factory& factory) {
        std::istringstream stream(serialized);
        std::string token;
        int version = 0;
        if (!(stream >> token >> version) || token != "RASGO_PATCH" || version != 2)
            throw std::invalid_argument("unsupported patch header");

        std::size_t nodeCount = 0;
        if (!(stream >> token >> nodeCount) || token != "NODES")
            throw std::invalid_argument("invalid node section");

        Graph graph;
        for (std::size_t index = 0; index < nodeCount; ++index) {
            std::size_t id = 0;
            std::string type;
            std::string state;
            std::string parameterState;
            if (!(stream >> token >> id >> type >> std::quoted(state)
                  >> std::quoted(parameterState)) || token != "NODE" || id != index)
                throw std::invalid_argument("invalid node entry");
            auto module = factory(type);
            if (!module)
                throw std::invalid_argument("factory returned null module");
            module->restoreState(state);
            module->restoreParameterState(parameterState);
            graph.add(std::move(module));
        }

        std::size_t connectionCount = 0;
        if (!(stream >> token >> connectionCount) || token != "CONNECTIONS")
            throw std::invalid_argument("invalid connection section");
        for (std::size_t index = 0; index < connectionCount; ++index) {
            Connection connection;
            if (!(stream >> token >> connection.source >> connection.sourcePort
                  >> connection.target >> connection.targetPort) || token != "CONNECT")
                throw std::invalid_argument("invalid connection entry");
            graph.connect(connection.source, connection.sourcePort,
                          connection.target, connection.targetPort);
        }
        return graph;
    }

private:
    Module& node(const std::size_t id) {
        if (id >= nodes_.size())
            throw std::out_of_range("node out of range");
        return *nodes_[id];
    }

    const Module& node(const std::size_t id) const {
        if (id >= nodes_.size())
            throw std::out_of_range("node out of range");
        return *nodes_[id];
    }

    std::vector<std::size_t> evaluationOrder() const {
        std::vector<std::vector<std::size_t>> outgoing(nodes_.size());
        for (const auto& connection : connections_)
            outgoing[connection.source].push_back(connection.target);

        std::vector<int> marks(nodes_.size(), 0);
        std::vector<std::size_t> order;
        std::function<void(std::size_t)> visit = [&](const std::size_t id) {
            if (marks[id] == 1)
                throw std::logic_error("graph cycle requires an explicit stateful scheduler");
            if (marks[id] == 2)
                return;
            marks[id] = 1;
            for (const auto target : outgoing[id])
                visit(target);
            marks[id] = 2;
            order.push_back(id);
        };
        for (std::size_t id = 0; id < nodes_.size(); ++id)
            visit(id);
        std::reverse(order.begin(), order.end());
        return order;
    }

    std::vector<std::unique_ptr<Module>> nodes_;
    std::vector<Connection> connections_;
};

} // namespace rasgo::modular
