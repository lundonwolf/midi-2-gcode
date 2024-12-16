#include "midi_parser.h"
#include "gcode_generator.h"
#include "file_dialog.h"
#include "app_settings.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
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
#include "gcode_visualizer.h"
#include "midi_player.h"

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

static std::unique_ptr<GCodeVisualizer> m_visualizer;
static std::unique_ptr<MidiPlayer> m_midiPlayer;
static bool m_showVisualizerWindow = true;
static bool m_showMidiPlayerWindow = true;
static float m_playbackTempo = 1.0f;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void updatePreview(const std::string& gcode) {
    previewText = gcode;
    showPreview = true;
}

bool convertMidiToGcode() {
    if (strlen(inputPath) == 0 || strlen(outputPath) == 0) {
        statusMessage = "Please select both input and output files.";
        return false;
    }

    try {
        std::vector<MidiNote> notes;
        MidiParser parser;
        if (!parser.parse(inputPath, notes)) {
            statusMessage = "Failed to parse MIDI file.";
            return false;
        }

        GCodeGenerator generator;
        if (m_visualizer) {
            generator.setVisualizer(m_visualizer.get());
        }

        generator.generateGCodeToFile(inputPath, outputPath);
        statusMessage = "Conversion successful!";
        return true;
    }
    catch (const std::exception& e) {
        statusMessage = "Error: " + std::string(e.what());
        return false;
    }
}

void renderSettingsWindow() {
    if (!showSettings) return;

    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", &showSettings)) {
        if (ImGui::CollapsingHeader("Appearance")) {
            static bool darkMode = AppSettings::getInstance().getDarkMode();
            if (ImGui::Checkbox("Dark Mode", &darkMode)) {
                AppSettings::getInstance().setDarkMode(darkMode);
                if (darkMode)
                    ImGui::StyleColorsDark();
                else
                    ImGui::StyleColorsLight();
            }
        }

        if (ImGui::CollapsingHeader("Printer Profiles")) {
            // List existing profiles
            auto& profiles = AppSettings::getInstance().getPrinterProfiles();
            static int selectedProfile = -1;

            for (size_t i = 0; i < profiles.size(); i++) {
                if (ImGui::Selectable(profiles[i].name.c_str(), selectedProfile == static_cast<int>(i))) {
                    selectedProfile = static_cast<int>(i);
                }
            }

            ImGui::Separator();

            // Add new profile
            ImGui::Text("Add New Profile");
            ImGui::InputText("Name", newPrinterName, sizeof(newPrinterName));
            ImGui::InputText("Manufacturer", newPrinterManufacturer, sizeof(newPrinterManufacturer));
            ImGui::InputDouble("Bed Size X (mm)", &newPrinterBedX, 1.0, 10.0);
            ImGui::InputDouble("Bed Size Y (mm)", &newPrinterBedY, 1.0, 10.0);
            ImGui::InputDouble("Max Speed (mm/s)", &newPrinterMaxSpeed, 1.0, 10.0);
            ImGui::InputDouble("Acceleration (mm/s²)", &newPrinterAccel, 10.0, 100.0);
            ImGui::InputDouble("Jerk (mm/s)", &newPrinterJerk, 0.1, 1.0);
            ImGui::InputDouble("Steps per mm", &newPrinterSteps, 1.0, 10.0);

            if (ImGui::Button("Add Profile")) {
                if (strlen(newPrinterName) > 0) {
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

                    // Clear input fields
                    memset(newPrinterName, 0, sizeof(newPrinterName));
                    memset(newPrinterManufacturer, 0, sizeof(newPrinterManufacturer));
                    newPrinterBedX = 220.0;
                    newPrinterBedY = 220.0;
                    newPrinterMaxSpeed = 200.0;
                    newPrinterAccel = 1000.0;
                    newPrinterJerk = 8.0;
                    newPrinterSteps = 80.0;
                }
            }

            // Delete selected profile
            if (selectedProfile >= 0 && ImGui::Button("Delete Selected Profile")) {
                AppSettings::getInstance().deleteCustomPrinter(selectedProfile);
                selectedProfile = -1;
            }
        }

        ImGui::End();
    }
}

void renderMainWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(mainWindowSize);
    ImGui::Begin("MIDI to G-code Converter", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

    // Menu Bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Settings")) {
                showSettings = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Main content area with columns
    const float columnWidth = ImGui::GetWindowContentRegionWidth() * 0.5f;
    
    // Left column: File selection and conversion
    ImGui::BeginChild("LeftColumn", ImVec2(columnWidth, 0), true);
    ImGui::Text("File Selection");
    ImGui::Separator();

    if (ImGui::Button("Select MIDI File")) {
        std::string file = FileDialog::OpenFile("MIDI Files\0*.mid;*.midi\0All Files\0*.*\0");
        if (!file.empty()) {
            strncpy_s(inputPath, file.c_str(), sizeof(inputPath) - 1);
        }
    }
    ImGui::SameLine();
    ImGui::Text(strlen(inputPath) > 0 ? inputPath : "No file selected");

    if (ImGui::Button("Select Output File")) {
        std::string file = FileDialog::SaveFile("G-code Files\0*.gcode\0All Files\0*.*\0");
        if (!file.empty()) {
            strncpy_s(outputPath, file.c_str(), sizeof(outputPath) - 1);
        }
    }
    ImGui::SameLine();
    ImGui::Text(strlen(outputPath) > 0 ? outputPath : "No file selected");

    // Convert Button
    if (ImGui::Button("Convert")) {
        if (convertMidiToGcode()) {
            ImGui::OpenPopup("Conversion Success");
        } else {
            ImGui::OpenPopup("Conversion Failed");
        }
    }

    // MIDI Player Section
    ImGui::Separator();
    ImGui::Text("MIDI Player");
    ImGui::Separator();
    
    // Add MIDI player controls here
    if (strlen(inputPath) > 0) {
        if (ImGui::Button("Play")) {
            // Add play functionality
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            // Add stop functionality
        }
    }
    
    ImGui::EndChild();

    // Right column: G-code Visualizer
    ImGui::SameLine();
    ImGui::BeginChild("RightColumn", ImVec2(0, 0), true);
    ImGui::Text("G-code Visualizer");
    ImGui::Separator();

    // Add G-code visualization here
    if (strlen(outputPath) > 0) {
        // Render the G-code preview
        ImGui::BeginChild("GCodeView", ImVec2(0, -30), true);
        // Add visualization rendering here
        ImGui::EndChild();

        // Controls below visualization
        if (ImGui::Button("Reset View")) {
            // Add reset view functionality
        }
        ImGui::SameLine();
        if (ImGui::Button("Load G-code")) {
            // Add load G-code functionality
        }
    } else {
        ImGui::Text("Convert a MIDI file to see the G-code preview");
    }
    
    ImGui::EndChild();

    // Conversion result popups
    if (ImGui::BeginPopupModal("Conversion Success", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("MIDI file converted successfully!");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Conversion Failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to convert MIDI file!");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Render settings window if open
    renderSettingsWindow();
    
    ImGui::End();
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

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

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

    m_visualizer = std::make_unique<GCodeVisualizer>();
    m_midiPlayer = std::make_unique<MidiPlayer>();
    if (!m_midiPlayer->initialize()) {
        std::cerr << "Failed to initialize MIDI playback system" << std::endl;
    }

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
