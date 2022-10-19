#pragma once

#include "../config.h"
#include "../QLogger.h"

#include "py/SnakeHandler.h"

#include <memory>
#include <vector>

class SDCommandsInterface {
private:
    std::unique_ptr<SnakeHandler> m_py_handle;

public:
    SDCommandsInterface();
    ~SDCommandsInterface();

    void TextToImage();

    void ImageToImage();
};