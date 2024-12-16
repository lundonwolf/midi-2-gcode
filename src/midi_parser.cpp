#include "midi_parser.h"
#include <fstream>
#include <iostream>
#include <cmath>

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

    size_t pos = 14;
    // Parse each track
    for (uint16_t i = 0; i < tracks && pos < data.size(); ++i) {
        if (data[pos] == 'M' && data[pos+1] == 'T' && 
            data[pos+2] == 'r' && data[pos+3] == 'k') {
            pos += 8; // Skip track header and length
            parseTrack(data, pos, ticksPerQuarterNote);
        }
    }

    return true;
}

void MidiParser::parseTrack(const std::vector<uint8_t>& data, size_t& pos, 
                           uint16_t ticksPerQuarterNote) {
    uint32_t tempo = 500000; // Default tempo (120 BPM)
    uint32_t absoluteTime = 0;
    std::vector<std::pair<uint8_t, uint32_t>> activeNotes;

    while (pos < data.size()) {
        uint32_t deltaTime = readVarLen(data, pos);
        absoluteTime += deltaTime;

        uint8_t status = data[pos++];
        if (status == 0xFF) { // Meta event
            uint8_t type = data[pos++];
            uint32_t length = readVarLen(data, pos);
            
            if (type == 0x51) { // Tempo change
                tempo = (data[pos] << 16) | (data[pos+1] << 8) | data[pos+2];
            }
            pos += length;
        }
        else if ((status & 0xF0) == 0x90) { // Note on
            uint8_t note = data[pos++];
            uint8_t velocity = data[pos++];
            
            if (velocity > 0) {
                activeNotes.push_back({note, absoluteTime});
            }
            else {
                // Note off (note-on with velocity 0)
                for (auto it = activeNotes.begin(); it != activeNotes.end(); ++it) {
                    if (it->first == note) {
                        MidiNote midiNote;
                        midiNote.note = note;
                        midiNote.velocity = velocity;
                        midiNote.timeOn = ticksToSeconds(it->second, ticksPerQuarterNote, tempo);
                        midiNote.timeOff = ticksToSeconds(absoluteTime, ticksPerQuarterNote, tempo);
                        notes.push_back(midiNote);
                        activeNotes.erase(it);
                        break;
                    }
                }
            }
        }
        else if ((status & 0xF0) == 0x80) { // Note off
            uint8_t note = data[pos++];
            uint8_t velocity = data[pos++];
            
            for (auto it = activeNotes.begin(); it != activeNotes.end(); ++it) {
                if (it->first == note) {
                    MidiNote midiNote;
                    midiNote.note = note;
                    midiNote.velocity = velocity;
                    midiNote.timeOn = ticksToSeconds(it->second, ticksPerQuarterNote, tempo);
                    midiNote.timeOff = ticksToSeconds(absoluteTime, ticksPerQuarterNote, tempo);
                    notes.push_back(midiNote);
                    activeNotes.erase(it);
                    break;
                }
            }
        }
        else {
            // Skip other events
            if ((status & 0xF0) >= 0x80) {
                pos += 2;
            }
        }
    }
}

uint32_t MidiParser::readVarLen(const std::vector<uint8_t>& data, size_t& pos) {
    uint32_t value = 0;
    uint8_t byte;
    
    do {
        byte = data[pos++];
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    
    return value;
}

double MidiParser::ticksToSeconds(uint32_t ticks, uint16_t ticksPerQuarterNote, uint32_t tempo) {
    return (ticks * tempo) / (ticksPerQuarterNote * 1000000.0);
}
