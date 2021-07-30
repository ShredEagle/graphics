#pragma once

#include "GenericDrawer.h"

#include <renderer/commons.h>

#include <glad/glad.h>

#include <boost/filesystem.hpp>

namespace ad {

class Chader
{
public:
    Chader();
    void loadProgram(const boost::filesystem::path & aVertexShader, const boost::filesystem::path & aFragmentShader);

    void render() const;

private:
    GenericDrawer mDrawer;
    VertexBufferObject mVertexData;
};

} // namespace ad
