#pragma once

#include "Shading.h"
#include "Texture.h"
#include "VertexSpecification.h"

namespace ad {

struct [[nodiscard]] DrawContext
{
    DrawContext(VertexSpecification aVertexSpecification,
                Program aProgram,
                std::vector<Texture> aTextures={}) :
        mVertexSpecification{std::move(aVertexSpecification)},
        mProgram{std::move(aProgram)},
        mTextures{std::move(aTextures)}
    {}

    VertexSpecification mVertexSpecification;
    Program  mProgram;
    std::vector<Texture>  mTextures;
};

} // namespace ad
