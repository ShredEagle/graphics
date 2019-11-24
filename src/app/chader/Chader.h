#pragma once

#include "GenericDrawer.h"

#include <renderer/commons.h>

#include <glad/glad.h>

namespace ad {

class Chader
{
public:
    Chader();
    void loadProgram(const path & aVertexShader, const path & aFragmentShader);
    void render() const;

private:
    GenericDrawer mDrawer;
    VertexBufferObject mVertexData;
};

} // namespace ad
