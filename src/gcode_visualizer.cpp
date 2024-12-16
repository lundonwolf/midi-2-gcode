#include "gcode_visualizer.h"
#include <glad/glad.h>
#include <sstream>
#include <regex>
#include <iostream>

static const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    uniform mat4 view;
    uniform mat4 proj;
    out vec3 fragColor;
    void main() {
        gl_Position = proj * view * vec4(aPos, 1.0);
        fragColor = aColor;
    }
)";

static const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(fragColor, 1.0);
    }
)";

GCodeVisualizer::GCodeVisualizer()
    : m_vao(0)
    , m_vbo(0)
    , m_shader(0)
    , m_scale(1.0f)
    , m_currentX(0.0f)
    , m_currentY(0.0f)
    , m_currentZ(0.0f)
    , m_isExtruding(false)
{
    m_center = glm::vec3(0.0f);
    
    initializeGL();
}

GCodeVisualizer::~GCodeVisualizer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_shader) glDeleteProgram(m_shader);
}

void GCodeVisualizer::initializeGL() {
    // Create and compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        return;
    }

    // Create and compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        return;
    }

    // Create shader program
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vertexShader);
    glAttachShader(m_shader, fragmentShader);
    glLinkProgram(m_shader);

    // Check shader program linking
    glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shader, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create VAO and VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    // Initialize view matrix
    m_view = glm::lookAt(
        glm::vec3(0.0f, -5.0f, 5.0f),  // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f),   // Look at point
        glm::vec3(0.0f, 0.0f, 1.0f)    // Up vector
    );

    // Initialize projection matrix
    float aspect = 1.0f;  // Will be updated when rendering
    m_proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
}

void GCodeVisualizer::loadGCode(const std::string& gcode) {
    m_lines.clear();
    parseGCode(gcode);
    updateBuffers();
}

void GCodeVisualizer::parseGCode(const std::string& gcode) {
    std::istringstream stream(gcode);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == ';') continue;
        
        std::smatch match;
        std::regex coords("([GXYZEF])(-?\\d*\\.?\\d+)");
        
        int command = -1;
        float x = m_currentX;
        float y = m_currentY;
        float z = m_currentZ;
        bool hasMove = false;
        
        std::string::const_iterator searchStart(line.cbegin());
        while (std::regex_search(searchStart, line.cend(), match, coords)) {
            char type = match[1].str()[0];
            float value = std::stof(match[2]);
            
            switch (type) {
                case 'G': command = static_cast<int>(value); break;
                case 'X': x = value; hasMove = true; break;
                case 'Y': y = value; hasMove = true; break;
                case 'Z': z = value; hasMove = true; break;
                case 'E': m_isExtruding = value > 0; break;
            }
            
            searchStart = match.suffix().first;
        }
        
        if (hasMove && (command == 0 || command == 1)) {
            glm::vec3 color = m_isExtruding ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
            m_lines.push_back({
                glm::vec3(m_currentX, m_currentY, m_currentZ),
                glm::vec3(x, y, z),
                color
            });
            
            m_currentX = x;
            m_currentY = y;
            m_currentZ = z;
        }
    }
}

void GCodeVisualizer::updateBuffers() {
    m_vertices.clear();
    for (const auto& line : m_lines) {
        m_vertices.push_back({line.start, line.color});
        m_vertices.push_back({line.end, line.color});
    }
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void GCodeVisualizer::render() {
    glUseProgram(m_shader);
    
    // Set uniforms
    int viewLoc = glGetUniformLocation(m_shader, "view");
    int projLoc = glGetUniformLocation(m_shader, "proj");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &m_proj[0][0]);
    
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, m_vertices.size());
}

void GCodeVisualizer::setViewMatrix(const glm::mat4& view) {
    m_view = view;
}

void GCodeVisualizer::setProjMatrix(const glm::mat4& proj) {
    m_proj = proj;
}

void GCodeVisualizer::resetView() {
    m_view = glm::lookAt(
        glm::vec3(0.0f, -200.0f, 100.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    m_scale = 1.0f;
}

void GCodeVisualizer::pan(float dx, float dy) {
    m_view = glm::translate(m_view, glm::vec3(dx, 0.0f, dy));
}

void GCodeVisualizer::rotate(float dx, float dy) {
    m_view = glm::rotate(m_view, glm::radians(dx), glm::vec3(0.0f, 0.0f, 1.0f));
    m_view = glm::rotate(m_view, glm::radians(dy), glm::vec3(1.0f, 0.0f, 0.0f));
}

void GCodeVisualizer::zoom(float delta) {
    m_scale *= (1.0f + delta);
    m_view = glm::scale(m_view, glm::vec3(m_scale));
}
