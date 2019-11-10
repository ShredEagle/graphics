#pragma once

#include "GenericDrawer.h"

#include <glad/glad.h>

#include <filesystem>

using std::filesystem::path;

namespace ad {

class Chader
{
public:
    void loadProgram(const path & aVertexShader, const path & aFragmentShader);

private:
    GenericDrawer mDrawer;
};

} // namespace ad
