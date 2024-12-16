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
    void setAcceleration(double acc) { acceleration = acc; }
    void setJerk(double j) { jerk = j; }
    
    // Generate G-code from MIDI notes
    std::string generateGCode(const std::vector<MidiNote>& notes);

private:
    double maxSpeed;    // Maximum speed for movements (mm/s)
    double stepsPerMm;  // Steps per millimeter for the stepper motor
    double acceleration;// Acceleration in mm/s²
    double jerk;       // Jerk in mm/s
    
    // Convert MIDI note to frequency
    double noteToFreq(uint8_t note);
    
    // Convert frequency to motor speed
    double freqToSpeed(double frequency);
    
    // Generate acceleration and deceleration commands
    std::string generateAccelCommand(double startSpeed, double endSpeed, double distance);
};
