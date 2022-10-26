#include "GridChunk.h"

GridChunk::GridChunk(int x, int y, int id) : c_coordinates{std::pair<int,int>(x,y)}, c_canvasId{id} {

}

GridChunk::~GridChunk() {

}

// Check if grid is currently visible based on world coordinates and window size
bool GridChunk::visible(const std::pair<int,int> &windowCoords, const std::pair<int,int> &windowSize) {
    // TODO: use simple box boundary check to see if 512x512 grid is within the window (offsetting the window for world coordinates)
    if (
        c_coordinates.first < windowCoords.first + windowSize.first &&
        c_coordinates.first + GRID_SIZE > windowCoords.first &&
        c_coordinates.second < windowCoords.second + windowSize.second &&
        c_coordinates.second + GRID_SIZE > windowCoords.second
    ) {
        return true;
    } else {
        return false;
    }
}

// Render onto screen, offset based on world coordinates & window size
void GridChunk::render(const std::pair<int,int> windowCoords) {

}