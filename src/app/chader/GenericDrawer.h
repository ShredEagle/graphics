#pragma once

#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

namespace ad {

struct GenericDrawer
{
    VertexArrayObject mVertexArray;
    Program mProgram;

    std::size_t mVertexCount{0};
    std::size_t mInstanceCount{1};
};

} // namespace ad
