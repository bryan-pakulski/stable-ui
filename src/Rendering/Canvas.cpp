#include "Canvas.h"

Canvas::~Canvas() {}

Canvas::Canvas(std::pair<int, int> coords, const std::string &name, GLFWwindow *w, std::shared_ptr<Camera> c) : BaseObject(coords), m_coords{coords}, m_name{name} {
    int success;
    m_window = w;
    m_camera = std::shared_ptr<Camera>(c);

    glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second);

    // Set vertex data
    float vertices[] = {
        // positions        // colors         // texture coords
        1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
        1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    };

    // Index buffer // Element Buffer Objects (EBO)
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    std::string vShaderSource = readShader("data/shaders/MainWindow_V.glsl");
    std::string fShaderSource = readShader("data/shaders/MainWindow_F.glsl");

    unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), success);
    unsigned int fragmentShader =  initFragmentShader(fShaderSource.c_str(), success);

    linkShaders(vertexShader, fragmentShader, success);
    setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices));

    int imgX = 0;
    int imgY = 0;
    GLHELPER::LoadTextureFromFile("data/images/uv_grid.png", &m_texture_id, &imgX, &imgY, true);
}

void Canvas::updateLogic() { 
    // Get updated screen size
    glfwGetFramebufferSize(m_window, &m_screen.first, &m_screen.second); 
}

void Canvas::updateVisual() {
    glUseProgram(shaderProgram);

    // Model code
    glm::mat4x4 model = glm::mat4x4(1.0f);                                      // translation
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 0.0, 1.0));  // rotation
    model = glm::scale(model, glm::vec3(1500, 1000, 1));                        // scale
    setMat4("model", model);

    // Projection code to fix object not scaling correctly on window resize
    setMat4("projection", m_camera->GetProjectionMatrix());

    // Update texture information
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // Check which chunks are in view and should be rendered
    for (auto &chunk : m_editorGrid) {
        if (chunk->visible(m_coords, m_screen)) {

        }
    }
}

void Canvas::setTexture(GLuint *id) { m_texture_id = *id; }