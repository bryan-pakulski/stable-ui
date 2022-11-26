#include "Chunk.h"

Chunk::Chunk(std::shared_ptr<Image> im, std::shared_ptr<Camera> c, int x, int y, int id) : m_image{im}, c_coordinates{std::pair<int,int>(x,y)}, BaseObject(std::pair<int,int>(x,y)) {
    int success;
    m_camera = std::shared_ptr<Camera>(c);

    // Vertex data
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

    std::string vShaderSource = readShader("data/shaders/Base_V.glsl");
    std::string fShaderSource = readShader("data/shaders/Base_F.glsl");

    unsigned int vertexShader = initVertexShader(vShaderSource.c_str(), success);
    unsigned int fragmentShader =  initFragmentShader(fShaderSource.c_str(), success);

    linkShaders(vertexShader, fragmentShader, success);
    setShaderBuffers(vertices, sizeof(vertices), indices, sizeof(indices));

    GLHELPER::LoadTextureFromFile(m_image->m_image_source.c_str(), &m_image->m_texture, &m_image->m_width, &m_image->m_height, true);

}

Chunk::~Chunk() {

}

// Check if grid is currently visible based on world coordinates and window size
bool Chunk::visible(const std::pair<int,int> &windowCoords, const std::pair<int,int> &windowSize) {
    // TODO: use simple box boundary check to see if 512x512 grid is within the window (offsetting the window for world coordinates)
    if (
        c_coordinates.first < windowCoords.first + windowSize.first &&
        c_coordinates.first + m_image->m_width > windowCoords.first &&
        c_coordinates.second < windowCoords.second + windowSize.second &&
        c_coordinates.second + m_image->m_height > windowCoords.second
    ) {
        return true;
    } else {
        return false;
    }
}

void Chunk::updateLogic() {

}

// Render onto screen, offset based on world coordinates & window size
void Chunk::updateVisual() {
    glUseProgram(shaderProgram);

    // View code
    setMat4("viewProjection", m_camera->getViewProjectionMatrix());

    // Model code, default canvas scale is 16,000 x 16,000 pixels
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)) *                                        // translation
            glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *                                   // rotation
            glm::scale(glm::mat4(1.0f), glm::vec3((float)m_image->m_width, (float)m_image->m_height, 1.0f));    // scale
    setMat4("model", model);

    // Update texture information
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, m_image->m_texture);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}