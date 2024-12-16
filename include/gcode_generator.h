#pragma once
#include "midi_parser.h"
#include <string>
#include <vector>

class GCodeGenerator {
public:
    GCodeGenerator();
    ~GCodeGenerator() = default;

    // Configure printer settings
    void setMaxSpeed(double speed) { maxSpeed = speed; }
    void setStepsPerMm(double steps) { stepsPerMm = steps; }
    
    // Generate G-code from MIDI notes
    std::string generateGCode(const std::vector<MidiNote>& notes);

private:
    double maxSpeed;    // Maximum speed for movements (mm/s)
    double stepsPerMm;  // Steps per millimeter for the stepper motor
    
    // Convert MIDI note to frequency
    double noteToFreq(uint8_t note);
    
    // Convert frequency to motor speed
    double freqToSpeed(double frequency);
};
