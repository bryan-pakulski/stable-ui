#include "Chunk.h"

Chunk::Chunk(int x, int y, int id) : c_coordinates{std::pair<int,int>(x,y)} {

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

// Render onto screen, offset based on world coordinates & window size
void Chunk::render(const std::pair<int,int> windowCoords) {

}