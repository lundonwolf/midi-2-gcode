#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GCodeVisualizer {
public:
    GCodeVisualizer();
    ~GCodeVisualizer();

    void loadGCode(const std::string& gcode);
    void render();
    void setViewMatrix(const glm::mat4& view);
    void setProjMatrix(const glm::mat4& proj);
    void resetView();
    void pan(float dx, float dy);
    void rotate(float dx, float dy);
    void zoom(float delta);

private:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    struct Line {
        glm::vec3 start;
        glm::vec3 end;
        glm::vec3 color;
    };

    void initializeGL();
    void parseGCode(const std::string& gcode);
    void updateBuffers();

    std::vector<Line> m_lines;
    std::vector<Vertex> m_vertices;
    
    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_shader;

    glm::mat4 m_view;
    glm::mat4 m_proj;
    glm::vec3 m_center;
    float m_scale;
    
    // Current position state for G-code parsing
    float m_currentX;
    float m_currentY;
    float m_currentZ;
    bool m_isExtruding;
};
