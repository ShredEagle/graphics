#pragma once

#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>
#include <gsl/span>

#include <cassert>

namespace ad {

struct GenericDrawer
{
    VertexArrayObject mVertexArray;
    Program mProgram;

    std::size_t mVertexCount{0};
    std::size_t mInstanceCount{1};

    template <class T_vertex>
    VertexBufferObject addVertexBuffer(
        const std::initializer_list<AttributeDescription> & aAttributes,
        gsl::span<T_vertex> mVertices)
    {
        if (mVertexCount)
        {
            assert(static_cast<std::size_t>(mVertices.size()) == mVertexCount);
        }
        else
        {
            mVertexCount = mVertices.size();
        }

        glBindVertexArray(mVertexArray);
        return makeLoadedVertexBuffer(aAttributes,
                                      sizeof(T_vertex),
                                      sizeof(T_vertex)*mVertices.size(),
                                      mVertices.data());
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

} // namespace ad
