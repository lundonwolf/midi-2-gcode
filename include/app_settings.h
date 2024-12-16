#pragma once
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

struct PrinterProfile {
    std::string name;
    std::string manufacturer;
    double bedSizeX;
    double bedSizeY;
    double maxSpeed;
    double acceleration;
    double jerk;
    double stepsPerMm;
    bool isCustom;
};

class AppSettings {
public:
    static AppSettings& getInstance() {
        static AppSettings instance;
        return instance;
    }

    void loadSettings();
    void saveSettings();
    
    // Getters
    const std::string& getOutputDirectory() const { return outputDirectory; }
    const std::vector<PrinterProfile>& getPrinterProfiles() const { return printerProfiles; }
    const PrinterProfile& getCurrentPrinter() const { return printerProfiles[currentPrinterIndex]; }
    bool getDarkMode() const { return darkMode; }
    
    // Setters
    void setOutputDirectory(const std::string& dir) { 
        outputDirectory = dir;
        saveSettings();
    }
    
    void setCurrentPrinter(size_t index) {
        if (index < printerProfiles.size()) {
            currentPrinterIndex = index;
            saveSettings();
        }
    }
    
    void setDarkMode(bool dark) {
        darkMode = dark;
        saveSettings();
    }
    
    void addCustomPrinter(const PrinterProfile& profile) {
        printerProfiles.push_back(profile);
        saveSettings();
    }
    
    void updateCustomPrinter(size_t index, const PrinterProfile& profile) {
        if (index < printerProfiles.size() && printerProfiles[index].isCustom) {
            printerProfiles[index] = profile;
            saveSettings();
        }
    }
    
    void deleteCustomPrinter(size_t index) {
        if (index < printerProfiles.size() && printerProfiles[index].isCustom) {
            printerProfiles.erase(printerProfiles.begin() + index);
            if (currentPrinterIndex >= printerProfiles.size()) {
                currentPrinterIndex = 0;
            }
            saveSettings();
        }
    }

private:
    AppSettings();  // Private constructor for singleton
    
    void initializeDefaultProfiles();
    std::string getSettingsPath() const;
    
    std::string outputDirectory;
    std::vector<PrinterProfile> printerProfiles;
    size_t currentPrinterIndex;
    bool darkMode;
};
