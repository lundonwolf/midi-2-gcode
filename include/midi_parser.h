#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct MidiNote {
    uint8_t note;      // MIDI note number (0-127)
    uint8_t velocity;  // Note velocity (0-127)
    double duration;   // Note duration in seconds
    double timestamp;  // Time offset from start in seconds
};

class MidiParser {
public:
    MidiParser() = default;
    ~MidiParser() = default;

    bool loadFile(const std::string& filename);
    bool parse(const std::string& filename, std::vector<MidiNote>& notes);
    const std::vector<MidiNote>& getNotes() const { return m_notes; }

private:
    std::vector<MidiNote> m_notes;
    double ticksToSeconds(uint32_t ticks, uint16_t ticksPerQuarterNote, uint32_t tempo);
    void parseTrack(const std::vector<uint8_t>& data, size_t& pos, uint16_t ticksPerQuarterNote);
    uint32_t readVarLen(const std::vector<uint8_t>& data, size_t& pos);
};
