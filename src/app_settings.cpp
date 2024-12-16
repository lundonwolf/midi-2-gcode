#include "app_settings.h"
#include <fstream>
#include <shlobj.h>
#include <filesystem>
#include <iostream>

AppSettings::AppSettings() 
    : currentPrinterIndex(0)
    , darkMode(true)
{
    // Set default output directory to Documents/MIDI2GCode
    char documentsPath[MAX_PATH];
    HRESULT result = SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documentsPath);
    if (SUCCEEDED(result)) {
        outputDirectory = std::string(documentsPath) + "\\MIDI2GCode";
    } else {
        outputDirectory = ".\\output";
    }
    
    // Create output directory if it doesn't exist
    std::filesystem::create_directories(outputDirectory);
    
    initializeDefaultProfiles();
    loadSettings();
}

void AppSettings::initializeDefaultProfiles() {
    printerProfiles = {
        // Prusa printers
        {"Prusa MK3S+", "Prusa Research", 250, 210, 200, 1000, 8, 100, false},
        {"Prusa Mini+", "Prusa Research", 180, 180, 180, 1000, 8, 100, false},
        
        // Creality printers
        {"Ender 3", "Creality", 220, 220, 180, 500, 8, 80, false},
        {"Ender 3 V2", "Creality", 220, 220, 200, 500, 8, 80, false},
        {"Ender 5", "Creality", 220, 220, 200, 500, 8, 80, false},
        {"CR-10", "Creality", 300, 300, 180, 500, 8, 80, false},
        
        // Other popular printers
        {"Voron 2.4", "Voron Design", 350, 350, 300, 3000, 10, 80, false},
        {"Rat Rig V-Core 3", "Rat Rig", 300, 300, 300, 3000, 10, 80, false},
        {"Artillery Sidewinder X1", "Artillery", 300, 300, 150, 1000, 8, 80, false},
        {"Flashforge Creator Pro", "Flashforge", 225, 145, 150, 1000, 8, 88, false}
    };
}

std::string AppSettings::getSettingsPath() const {
    char appDataPath[MAX_PATH];
    HRESULT result = SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath);
    if (SUCCEEDED(result)) {
        std::string settingsDir = std::string(appDataPath) + "\\MIDI2GCode";
        std::filesystem::create_directories(settingsDir);
        return settingsDir + "\\settings.json";
    }
    return "settings.json";
}

void AppSettings::loadSettings() {
    try {
        std::cout << "Loading settings from: " << getSettingsPath() << std::endl;
        std::cout << "Number of default profiles: " << printerProfiles.size() << std::endl;
        
        std::ifstream file(getSettingsPath());
        if (file.is_open()) {
            nlohmann::json j;
            file >> j;
            
            if (j.contains("outputDirectory")) {
                outputDirectory = j["outputDirectory"];
                std::cout << "Loaded output directory: " << outputDirectory << std::endl;
            }
            if (j.contains("darkMode")) {
                darkMode = j["darkMode"];
            }
            if (j.contains("currentPrinterIndex")) {
                currentPrinterIndex = j["currentPrinterIndex"];
                std::cout << "Current printer index: " << currentPrinterIndex << std::endl;
            }
            if (j.contains("customPrinters")) {
                for (const auto& printer : j["customPrinters"]) {
                    PrinterProfile profile;
                    profile.name = printer["name"];
                    profile.manufacturer = printer["manufacturer"];
                    profile.bedSizeX = printer["bedSizeX"];
                    profile.bedSizeY = printer["bedSizeY"];
                    profile.maxSpeed = printer["maxSpeed"];
                    profile.acceleration = printer["acceleration"];
                    profile.jerk = printer["jerk"];
                    profile.stepsPerMm = printer["stepsPerMm"];
                    profile.isCustom = true;
                    printerProfiles.push_back(profile);
                    std::cout << "Loaded custom printer: " << profile.name << std::endl;
                }
            }
        } else {
            std::cout << "Settings file not found, using defaults" << std::endl;
        }
        std::cout << "Total number of profiles after loading: " << printerProfiles.size() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error loading settings: " << e.what() << std::endl;
    }
}

void AppSettings::saveSettings() {
    try {
        nlohmann::json j;
        j["outputDirectory"] = outputDirectory;
        j["darkMode"] = darkMode;
        j["currentPrinterIndex"] = currentPrinterIndex;
        
        // Save custom printers
        nlohmann::json customPrinters = nlohmann::json::array();
        for (const auto& printer : printerProfiles) {
            if (printer.isCustom) {
                nlohmann::json p;
                p["name"] = printer.name;
                p["manufacturer"] = printer.manufacturer;
                p["bedSizeX"] = printer.bedSizeX;
                p["bedSizeY"] = printer.bedSizeY;
                p["maxSpeed"] = printer.maxSpeed;
                p["acceleration"] = printer.acceleration;
                p["jerk"] = printer.jerk;
                p["stepsPerMm"] = printer.stepsPerMm;
                customPrinters.push_back(p);
            }
        }
        j["customPrinters"] = customPrinters;
        
        std::ofstream file(getSettingsPath());
        file << j.dump(4);
    } catch (const std::exception& e) {
        // Handle saving error
    }
}
