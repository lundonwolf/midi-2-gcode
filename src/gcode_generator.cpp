#include "gcode_generator.h"
#include <sstream>
#include <cmath>
#include <algorithm>

GCodeGenerator::GCodeGenerator() 
    : maxSpeed(200.0)    // Default 200mm/s
    , stepsPerMm(80.0)   // Default 80 steps/mm
{}

double GCodeGenerator::noteToFreq(uint8_t note) {
    // A4 = 69, 440Hz
    return 440.0 * std::pow(2.0, (note - 69.0) / 12.0);
}

double GCodeGenerator::freqToSpeed(double frequency) {
    // Convert frequency to motor speed
    // This is a simplified conversion - you might want to adjust this
    // based on your specific printer's characteristics
    return std::min(maxSpeed, frequency / 10.0);
}

std::string GCodeGenerator::generateGCode(const std::vector<MidiNote>& notes) {
    std::stringstream gcode;
    
    // Initial setup
    gcode << "G21 ; Set units to millimeters\n"
          << "G90 ; Use absolute coordinates\n"
          << "M83 ; Use relative distances for extrusion\n"
          << "M104 S0 ; Turn off hotend\n"
          << "M140 S0 ; Turn off heated bed\n\n";

    // Home all axes
    gcode << "G28 ; Home all axes\n\n";

    double currentX = 0.0;
    double currentTime = 0.0;

    for (const auto& note : notes) {
        // Wait if there's a gap
        if (note.timeOn > currentTime) {
            double waitTime = note.timeOn - currentTime;
            gcode << "G4 P" << static_cast<int>(waitTime * 1000) << " ; Pause for " << waitTime << " seconds\n";
        }

        // Calculate movement parameters
        double frequency = noteToFreq(note.note);
        double speed = freqToSpeed(frequency);
        double duration = note.timeOff - note.timeOn;
        double distance = speed * duration;

        // Ensure we stay within printer bounds by moving back and forth
        if (currentX + distance > 200.0) { // Assuming 200mm bed size
            currentX = 0.0;
            gcode << "G0 X0 F" << static_cast<int>(maxSpeed * 60) << " ; Return to start\n";
        }

        // Generate movement command
        gcode << "G1 X" << currentX + distance 
              << " F" << static_cast<int>(speed * 60) << " ; Play note " << (int)note.note 
              << " at " << frequency << "Hz\n";

        currentX += distance;
        currentTime = note.timeOff;
    }

    // Return to home position
    gcode << "\nG0 X0 F" << static_cast<int>(maxSpeed * 60) << " ; Return to start\n"
          << "M84 ; Disable motors\n";

    return gcode.str();
}
