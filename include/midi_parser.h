#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct MidiNote {
    uint8_t note;      // MIDI note number
    uint8_t velocity;  // Note velocity
    double timeOn;     // Time when note starts (in seconds)
    double timeOff;    // Time when note ends (in seconds)
};

class MidiParser {
public:
    MidiParser() = default;
    ~MidiParser() = default;

    bool loadFile(const std::string& filename);
    const std::vector<MidiNote>& getNotes() const { return notes; }

private:
    std::vector<MidiNote> notes;
    double ticksToSeconds(uint32_t ticks, uint16_t ticksPerQuarterNote, uint32_t tempo);
    void parseTrack(const std::vector<uint8_t>& data, size_t& pos, uint16_t ticksPerQuarterNote);
    uint32_t readVarLen(const std::vector<uint8_t>& data, size_t& pos);
};
