#pragma once
#include "midi_parser.h"
#include "gcode_visualizer.h"
#include <string>
#include <vector>
#include <fstream>

class GCodeGenerator {
public:
    GCodeGenerator();
    ~GCodeGenerator() = default;

    // Set parameters
    void setMaxSpeed(double speed) { maxSpeed = speed; }
    void setStepsPerMm(double steps) { stepsPerMm = steps; }
    void setAcceleration(double acc) { acceleration = acc; }
    void setJerk(double j) { jerk = j; }
    void setVisualizer(GCodeVisualizer* visualizer) { m_visualizer = visualizer; }

    // Generate G-code from MIDI notes
    std::string generateGCode(const std::vector<MidiNote>& notes);
    
    // Generate G-code and save to file
    void generateGCodeToFile(const std::string& inputFile, const std::string& outputFile);

private:
    double maxSpeed;    // Maximum speed for movements (mm/s)
    double stepsPerMm;  // Steps per millimeter for the stepper motor
    double acceleration; // Acceleration in mm/s²
    double jerk;       // Jerk in mm/s
    double bedSizeX;   // Bed size in X direction (mm)
    double bedSizeY;   // Bed size in Y direction (mm)
    GCodeVisualizer* m_visualizer;
    
    // Convert MIDI note to frequency
    double noteToFreq(uint8_t note);
    
    // Convert frequency to motor speed
    double freqToSpeed(double frequency);
    
    // Generate acceleration and deceleration commands
    std::string generateAccelCommand(double startSpeed, double endSpeed, double distance);
};
