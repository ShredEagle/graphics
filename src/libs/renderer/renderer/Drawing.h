#pragma once

#include "Shading.h"
#include "Texture.h"
#include "VertexSpecification.h"

namespace ad {
namespace graphics {


/// \deprecated This is too integrated and makes too many assumptions for a low-level library.
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

inline DrawContext makeBareContext()
{
    return DrawContext{VertexSpecification{}, Program{}};
}


inline void bindVertexArray(const DrawContext & aDrawContext)
{
    bind(aDrawContext.mVertexSpecification);
}


inline void useProgram(const DrawContext & aDrawContext)
{
    use(aDrawContext.mProgram);
}


inline void activate(const VertexArrayObject & aVertexArray, const Program & aProgram)
{
    bind(aVertexArray);
    use(aProgram);
}

// TODO Ad 2022/02/02: Is it a good idea to "expect" the object to unbind
// when the underlying unbinding mechanism does not use it (just reset a default)? 
inline void deactivate(const VertexArrayObject & aVertexArray, const Program &)
{
    unbind(aVertexArray);
    disableProgram();
}


inline void activate(const VertexSpecification & aVertexSpecification, const Program & aProgram)
{
    activate(aVertexSpecification.mVertexArray, aProgram);
}

inline void deactivate(const VertexSpecification & aVertexSpecification, const Program & aProgram)
{
    deactivate(aVertexSpecification.mVertexArray, aProgram);
}


inline void activate(const DrawContext & aDrawContext)
{
    activate(aDrawContext.mVertexSpecification, aDrawContext.mProgram);
}

inline void deactivate(const DrawContext & aDrawContext)
{
    deactivate(aDrawContext.mVertexSpecification, aDrawContext.mProgram);
}


inline std::vector<VertexBufferObject> & buffers(DrawContext & aDrawContext)
{
    return aDrawContext.mVertexSpecification.mVertexBuffers;
}


} // namespace graphics
} // namespace ad
