#include "midi_parser.h"
#include "gcode_generator.h"
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

// Global state
static char inputPath[256] = "";
static char outputPath[256] = "";
static bool conversionSuccess = false;
static std::string statusMessage;
static float maxSpeed = 200.0f;
static float stepsPerMm = 80.0f;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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

    std::string gcode = generator.generateGCode(parser.getNotes());

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "MIDI to G-code Converter", nullptr, nullptr);
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
    ImGui::StyleColorsDark();

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

        // Create the main window
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("MIDI to G-code Converter", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // File selection
        ImGui::Text("Input MIDI File:");
        ImGui::InputText("##input", inputPath, IM_ARRAYSIZE(inputPath));
        ImGui::SameLine();
        if (ImGui::Button("Browse##1")) {
            // TODO: Add file dialog
        }

        ImGui::Text("Output G-code File:");
        ImGui::InputText("##output", outputPath, IM_ARRAYSIZE(outputPath));
        ImGui::SameLine();
        if (ImGui::Button("Browse##2")) {
            // TODO: Add file dialog
        }

        // Settings
        ImGui::Separator();
        ImGui::Text("Printer Settings:");
        ImGui::SliderFloat("Max Speed (mm/s)", &maxSpeed, 50.0f, 500.0f);
        ImGui::SliderFloat("Steps per mm", &stepsPerMm, 20.0f, 200.0f);

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
