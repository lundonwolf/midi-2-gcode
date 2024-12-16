#include "midi_parser.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

bool MidiParser::parse(const std::string& filename, std::vector<MidiNote>& notes) {
    if (!notes.empty()) {
        notes.clear();
    }
    if (!loadFile(filename)) {
        return false;
    }
    notes = m_notes;
    std::sort(notes.begin(), notes.end(), [](const MidiNote& a, const MidiNote& b) {
        return a.timestamp < b.timestamp;
    });
    return true;
}

bool MidiParser::loadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return false;
    }

    // Read entire file into vector
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    file.close();

    // Check MIDI header
    if (data.size() < 14 || 
        data[0] != 'M' || data[1] != 'T' || 
        data[2] != 'h' || data[3] != 'd') {
        std::cerr << "Invalid MIDI file format" << std::endl;
        return false;
    }

    // Parse header
    uint16_t format = (data[8] << 8) | data[9];
    uint16_t tracks = (data[10] << 8) | data[11];
    uint16_t ticksPerQuarterNote = (data[12] << 8) | data[13];

    m_notes.clear();
    size_t pos = 14;

    // Parse each track
    for (uint16_t i = 0; i < tracks && pos < data.size(); ++i) {
        if (pos + 8 >= data.size()) break;
        
        if (data[pos] == 'M' && data[pos+1] == 'T' && 
            data[pos+2] == 'r' && data[pos+3] == 'k') {
            
            // Get track length
            uint32_t trackLength = (data[pos+4] << 24) | (data[pos+5] << 16) | 
                                 (data[pos+6] << 8) | data[pos+7];
            pos += 8;
            
            if (pos + trackLength <= data.size()) {
                parseTrack(data, pos, ticksPerQuarterNote);
            }
        } else {
            // Skip unknown chunk
            if (pos + 4 < data.size()) {
                uint32_t length = (data[pos+4] << 24) | (data[pos+5] << 16) | 
                                (data[pos+6] << 8) | data[pos+7];
                pos += 8 + length;
            } else {
                break;
            }
        }
    }

    return true;
}

void MidiParser::parseTrack(const std::vector<uint8_t>& data, size_t& pos, 
                           uint16_t ticksPerQuarterNote) {
    uint32_t tempo = 500000; // Default tempo (120 BPM)
    uint32_t absoluteTime = 0;
    std::map<uint8_t, uint32_t> noteStarts; // note -> start time

    while (pos < data.size()) {
        uint32_t deltaTime = readVarLen(data, pos);
        absoluteTime += deltaTime;

        if (pos >= data.size()) break;
        
        uint8_t status = data[pos++];
        
        if (status == 0xFF) { // Meta event
            if (pos >= data.size()) break;
            uint8_t type = data[pos++];
            uint32_t length = readVarLen(data, pos);
            
            if (type == 0x51 && length == 3 && pos + 2 < data.size()) { // Tempo change
                tempo = (data[pos] << 16) | (data[pos+1] << 8) | data[pos+2];
            }
            pos += length;
        }
        else if ((status & 0xF0) == 0x90) { // Note on
            if (pos + 1 >= data.size()) break;
            
            uint8_t note = data[pos++];
            uint8_t velocity = data[pos++];
            
            if (velocity > 0) {
                noteStarts[note] = absoluteTime;
            } else {
                // Note off (note-on with velocity 0)
                auto it = noteStarts.find(note);
                if (it != noteStarts.end()) {
                    MidiNote midiNote;
                    midiNote.note = note;
                    midiNote.velocity = velocity;
                    midiNote.timestamp = ticksToSeconds(it->second, ticksPerQuarterNote, tempo);
                    midiNote.duration = ticksToSeconds(absoluteTime - it->second, ticksPerQuarterNote, tempo);
                    m_notes.push_back(midiNote);
                    noteStarts.erase(it);
                }
            }
        }
        else if ((status & 0xF0) == 0x80) { // Note off
            if (pos + 1 >= data.size()) break;
            
            uint8_t note = data[pos++];
            uint8_t velocity = data[pos++];
            
            auto it = noteStarts.find(note);
            if (it != noteStarts.end()) {
                MidiNote midiNote;
                midiNote.note = note;
                midiNote.velocity = velocity;
                midiNote.timestamp = ticksToSeconds(it->second, ticksPerQuarterNote, tempo);
                midiNote.duration = ticksToSeconds(absoluteTime - it->second, ticksPerQuarterNote, tempo);
                m_notes.push_back(midiNote);
                noteStarts.erase(it);
            }
        }
        else if ((status & 0x80) == 0) { // Running status
            pos--; // Rewind to reread the data byte
            // Handle running status here if needed
        }
    }
    
    // Close any open notes at the end of track
    for (const auto& [note, startTime] : noteStarts) {
        MidiNote midiNote;
        midiNote.note = note;
        midiNote.velocity = 0;
        midiNote.timestamp = ticksToSeconds(startTime, ticksPerQuarterNote, tempo);
        midiNote.duration = ticksToSeconds(absoluteTime - startTime, ticksPerQuarterNote, tempo);
        m_notes.push_back(midiNote);
    }
}

double MidiParser::ticksToSeconds(uint32_t ticks, uint16_t ticksPerQuarterNote, uint32_t tempo) {
    return (ticks * tempo) / (ticksPerQuarterNote * 1000000.0);
}

uint32_t MidiParser::readVarLen(const std::vector<uint8_t>& data, size_t& pos) {
    uint32_t value = 0;
    uint8_t byte;
    
    do {
        if (pos >= data.size()) return 0;
        byte = data[pos++];
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    
    return value;
}
