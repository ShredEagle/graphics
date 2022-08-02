#pragma once

#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

#include <cassert>

namespace ad {
namespace graphics {

struct GenericDrawer
{
    VertexArrayObject mVertexArray;
    Program mProgram;

    std::size_t mVertexCount{0};
    std::size_t mInstanceCount{1};

    template <class T_vertex>
    VertexBufferObject addVertexBuffer(
        const std::initializer_list<AttributeDescription> & aAttributes,
        std::span<T_vertex> aVertices)
    {
        if (mVertexCount)
        {
            assert(static_cast<std::size_t>(aVertices.size()) == mVertexCount);
        }
        else
        {
            mVertexCount = aVertices.size();
        }

        return loadVertexBuffer(mVertexArray,
                                aAttributes,
                                sizeof(T_vertex),
                                sizeof(T_vertex)*aVertices.size(),
                                aVertices.data());
    }

    void render() const
    {
        glBindVertexArray(mVertexArray);
        glUseProgram(mProgram);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                              0,
                              mVertexCount,
                              mInstanceCount);
    }
};

} // namespace graphics
} // namespace ad
