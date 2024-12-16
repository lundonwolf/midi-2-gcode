#include "midi_parser.h"
#include "gcode_generator.h"
#include "file_dialog.h"
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
static float maxSpeed = 200.0f;
static float stepsPerMm = 80.0f;
static float acceleration = 1000.0f;
static float jerk = 10.0f;
static bool useAcceleration = true;
static bool useJerk = true;
static std::string previewText;
static bool showPreview = false;
static ImVec2 mainWindowSize(1024, 768);
static bool darkTheme = true;

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

    // Parse MIDI file
    MidiParser parser;
    if (!parser.loadFile(inputPath)) {
        statusMessage = "Failed to load MIDI file: " + std::string(inputPath);
        return false;
    }

    // Generate G-code
    GCodeGenerator generator;
    generator.setMaxSpeed(maxSpeed);
    generator.setStepsPerMm(stepsPerMm);
    
    // Set additional parameters
    if (useAcceleration) {
        generator.setAcceleration(acceleration);
    }
    if (useJerk) {
        generator.setJerk(jerk);
    }

    std::string gcode = generator.generateGCode(parser.getNotes());
    
    // Update preview
    updatePreview(gcode);

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

void renderMainWindow() {
    ImGui::SetNextWindowSize(mainWindowSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("MIDI to G-code Converter", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // File selection
    ImGui::Text("Input MIDI File:");
    ImGui::InputText("##input", inputPath, IM_ARRAYSIZE(inputPath), ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse##1")) {
        std::string file = FileDialog::OpenFile("MIDI Files\0*.mid\0All Files\0*.*\0");
        if (!file.empty()) {
            strcpy_s(inputPath, file.c_str());
        }
    }

    ImGui::Text("Output G-code File:");
    ImGui::InputText("##output", outputPath, IM_ARRAYSIZE(outputPath), ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse##2")) {
        std::string file = FileDialog::SaveFile("G-code Files\0*.gcode\0All Files\0*.*\0");
        if (!file.empty()) {
            strcpy_s(outputPath, file.c_str());
        }
    }

    // Settings
    ImGui::Separator();
    ImGui::Text("Printer Settings:");
    
    if (ImGui::CollapsingHeader("Basic Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Max Speed (mm/s)", &maxSpeed, 50.0f, 500.0f);
        ImGui::SliderFloat("Steps per mm", &stepsPerMm, 20.0f, 200.0f);
    }
    
    if (ImGui::CollapsingHeader("Advanced Settings")) {
        ImGui::Checkbox("Use Acceleration", &useAcceleration);
        if (useAcceleration) {
            ImGui::SliderFloat("Acceleration (mm/s²)", &acceleration, 100.0f, 3000.0f);
        }
        
        ImGui::Checkbox("Use Jerk", &useJerk);
        if (useJerk) {
            ImGui::SliderFloat("Jerk (mm/s)", &jerk, 1.0f, 30.0f);
        }
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

    ImGui::End();

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
    GLFWwindow* window = glfwCreateWindow(mainWindowSize.x, mainWindowSize.y, 
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
    if (darkTheme)
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
