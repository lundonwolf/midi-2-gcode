#include "midi_parser.h"
#include "gcode_generator.h"
#include "file_dialog.h"
#include "app_settings.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>

// Global state
static char inputPath[256] = "";
static char outputPath[256] = "";
static bool conversionSuccess = false;
static std::string statusMessage;
static std::string previewText;
static bool showPreview = false;
static bool showSettings = false;
static ImVec2 mainWindowSize(1024, 768);

// Custom printer editor state
static char newPrinterName[128] = "";
static char newPrinterManufacturer[128] = "";
static double newPrinterBedX = 220.0;
static double newPrinterBedY = 220.0;
static double newPrinterMaxSpeed = 200.0;
static double newPrinterAccel = 1000.0;
static double newPrinterJerk = 8.0;
static double newPrinterSteps = 80.0;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void updatePreview(const std::string& gcode) {
    previewText = gcode;
    showPreview = true;
}

bool convertMidiToGcode() {
    if (strlen(inputPath) == 0) {
        statusMessage = "Please select an input MIDI file.";
        return false;
    }

    // Get current printer profile
    const PrinterProfile& printer = AppSettings::getInstance().getCurrentPrinter();
    
    // Parse MIDI file
    MidiParser parser;
    if (!parser.loadFile(inputPath)) {
        statusMessage = "Failed to load MIDI file: " + std::string(inputPath);
        return false;
    }

    // Generate output path if not specified
    if (strlen(outputPath) == 0) {
        std::filesystem::path inputFilePath(inputPath);
        std::string outputDir = AppSettings::getInstance().getOutputDirectory();
        std::string fileName = inputFilePath.stem().string() + ".gcode";
        std::filesystem::path outputFilePath = std::filesystem::path(outputDir) / fileName;
        strcpy_s(outputPath, outputFilePath.string().c_str());
    }

    // Generate G-code
    GCodeGenerator generator;
    generator.setMaxSpeed(printer.maxSpeed);
    generator.setStepsPerMm(printer.stepsPerMm);
    generator.setAcceleration(printer.acceleration);
    generator.setJerk(printer.jerk);

    std::string gcode = generator.generateGCode(parser.getNotes());
    
    // Update preview
    updatePreview(gcode);

    // Create output directory if it doesn't exist
    std::filesystem::path outputFilePath(outputPath);
    std::filesystem::create_directories(outputFilePath.parent_path());

    // Write G-code to file
    std::ofstream outFile(outputPath);
    if (!outFile) {
        statusMessage = "Failed to create output file: " + std::string(outputPath);
        return false;
    }

    outFile << gcode;
    outFile.close();

    statusMessage = "Successfully converted " + std::string(inputPath) + " to " + std::string(outputPath);
    return true;
}

void renderSettingsWindow() {
    if (!showSettings) return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", &showSettings)) {
        AppSettings& settings = AppSettings::getInstance();
        
        // Output directory
        if (ImGui::CollapsingHeader("Output Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            char outDir[256];
            strcpy_s(outDir, settings.getOutputDirectory().c_str());
            ImGui::InputText("Output Directory", outDir, sizeof(outDir), ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();
            if (ImGui::Button("Browse##outdir")) {
                std::string dir = FileDialog::OpenFile("Select Directory\0*.*\0");
                if (!dir.empty()) {
                    settings.setOutputDirectory(dir);
                }
            }
        }

        // Printer profiles
        if (ImGui::CollapsingHeader("Printer Profiles", ImGuiTreeNodeFlags_DefaultOpen)) {
            const auto& profiles = settings.getPrinterProfiles();
            static int currentItem = settings.getCurrentPrinter().isCustom ? 
                profiles.size() - 1 : 0;

            if (ImGui::BeginCombo("Current Printer", profiles[currentItem].name.c_str())) {
                for (int i = 0; i < profiles.size(); i++) {
                    const bool isSelected = (currentItem == i);
                    if (ImGui::Selectable(profiles[i].name.c_str(), isSelected)) {
                        currentItem = i;
                        settings.setCurrentPrinter(i);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Add custom printer
            if (ImGui::Button("Add Custom Printer")) {
                ImGui::OpenPopup("Add Custom Printer");
                memset(newPrinterName, 0, sizeof(newPrinterName));
                memset(newPrinterManufacturer, 0, sizeof(newPrinterManufacturer));
            }
        }

        // Theme settings
        if (ImGui::CollapsingHeader("Appearance")) {
            bool darkMode = settings.getDarkMode();
            if (ImGui::Checkbox("Dark Mode", &darkMode)) {
                settings.setDarkMode(darkMode);
                if (darkMode)
                    ImGui::StyleColorsDark();
                else
                    ImGui::StyleColorsLight();
            }
        }

        // Custom printer popup
        if (ImGui::BeginPopupModal("Add Custom Printer", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Printer Name", newPrinterName, sizeof(newPrinterName));
            ImGui::InputText("Manufacturer", newPrinterManufacturer, sizeof(newPrinterManufacturer));
            ImGui::InputDouble("Bed Size X (mm)", &newPrinterBedX, 1.0, 10.0);
            ImGui::InputDouble("Bed Size Y (mm)", &newPrinterBedY, 1.0, 10.0);
            ImGui::InputDouble("Max Speed (mm/s)", &newPrinterMaxSpeed, 1.0, 10.0);
            ImGui::InputDouble("Acceleration (mm/s²)", &newPrinterAccel, 10.0, 100.0);
            ImGui::InputDouble("Jerk (mm/s)", &newPrinterJerk, 0.1, 1.0);
            ImGui::InputDouble("Steps per mm", &newPrinterSteps, 1.0, 10.0);

            if (ImGui::Button("Add", ImVec2(120, 0))) {
                PrinterProfile profile;
                profile.name = newPrinterName;
                profile.manufacturer = newPrinterManufacturer;
                profile.bedSizeX = newPrinterBedX;
                profile.bedSizeY = newPrinterBedY;
                profile.maxSpeed = newPrinterMaxSpeed;
                profile.acceleration = newPrinterAccel;
                profile.jerk = newPrinterJerk;
                profile.stepsPerMm = newPrinterSteps;
                profile.isCustom = true;
                
                AppSettings::getInstance().addCustomPrinter(profile);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void renderMainWindow() {
    // Make the window fill the entire viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGui::Begin("MIDI to G-code Converter", nullptr, 
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open MIDI File")) {
                std::string file = FileDialog::OpenFile("MIDI Files\0*.mid\0All Files\0*.*\0");
                if (!file.empty()) {
                    strcpy_s(inputPath, file.c_str());
                    outputPath[0] = '\0'; // Clear output path to use auto-generated one
                }
            }
            if (ImGui::MenuItem("Settings")) {
                showSettings = true;
            }
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), 1);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Main content
    float menuBarHeight = ImGui::GetFrameHeight();
    ImGui::SetCursorPosY(menuBarHeight);

    // Create a child window for scrolling
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    // File selection
    ImGui::Text("Input MIDI File:");
    ImGui::InputText("##input", inputPath, IM_ARRAYSIZE(inputPath), ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse##1")) {
        std::string file = FileDialog::OpenFile("MIDI Files\0*.mid\0All Files\0*.*\0");
        if (!file.empty()) {
            strcpy_s(inputPath, file.c_str());
            outputPath[0] = '\0'; // Clear output path to use auto-generated one
        }
    }

    ImGui::Text("Output G-code File (optional):");
    ImGui::InputText("##output", outputPath, IM_ARRAYSIZE(outputPath), ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse##2")) {
        std::string file = FileDialog::SaveFile("G-code Files\0*.gcode\0All Files\0*.*\0");
        if (!file.empty()) {
            strcpy_s(outputPath, file.c_str());
        }
    }

    // Current printer info
    ImGui::Separator();
    const PrinterProfile& currentPrinter = AppSettings::getInstance().getCurrentPrinter();
    ImGui::Text("Current Printer: %s by %s", 
                currentPrinter.name.c_str(), 
                currentPrinter.manufacturer.c_str());
    if (ImGui::Button("Change Printer Settings")) {
        showSettings = true;
    }

    // Convert button
    ImGui::Separator();
    if (ImGui::Button("Convert", ImVec2(120, 30))) {
        conversionSuccess = convertMidiToGcode();
    }

    // Status message
    if (!statusMessage.empty()) {
        ImGui::Separator();
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }

    ImGui::EndChild();

    ImGui::End();

    // Settings window
    renderSettingsWindow();

    // G-code Preview Window
    if (showPreview && !previewText.empty()) {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("G-code Preview", &showPreview);
        ImGui::TextWrapped("Preview of generated G-code:");
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
        ImGui::TextUnformatted(previewText.c_str());
        ImGui::EndChild();
        if (ImGui::Button("Close")) {
            showPreview = false;
        }
        ImGui::End();
    }
}

// Windows entry point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(mainWindowSize.x), 
                                        static_cast<int>(mainWindowSize.y), 
                                        "MIDI to G-code Converter", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    if (AppSettings::getInstance().getDarkMode())
        ImGui::StyleColorsDark();
    else
        ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderMainWindow();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
