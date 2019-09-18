#include "Engine.h"

#include "shaders.h"

#include <handy/vector_utils.h>

#include <resource/PathProvider.h>

#include <renderer/Texture.h>

#include <math/Range.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

namespace ad {

using json = nlohmann::json;


//
// Vertex
//
struct Vertex
{
    math::Vec4<GLfloat> mPosition;
    math::Vec2<GLint> mUV;
};

// Note: texture_2D_rect indices are texel based (not normalized)
// the UV value should be computed from shader and frame size
// 10 is a quick fix
constexpr size_t gVerticesCount{4};
Vertex gVerticesQuad[gVerticesCount] = {
    {
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0, 0},
    },
    {
        {0.0f,  1.0f, 0.0f, 1.0f},
        {0, 1},
    },
    {
        { 1.0f, 0.0f, 0.0f, 1.0f},
        {1, 0},
    },
    {
        { 1.0f,  1.0f, 0.0f, 1.0f},
        {1, 1},
    },
};


/// \todo move this kind of feature to renderer
DrawContext makeBareContext()
{
    return DrawContext{VertexSpecification{}, Program{}};
}

Engine::Engine() :
    mDrawContext(makeBareContext()),
    mWindowSize(0, 0)
{
    //
    // General OpenGL setups
    //

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Frame buffer clear color
    glClearColor(0.1f, 0.2f, 0.3f, 1.f);


    //
    // Vertex description
    //
    glBindVertexArray(mDrawContext.mVertexSpecification.mVertexArray);

    // Per-vertex attributes
    mDrawContext.mVertexSpecification.mVertexBuffers.emplace_back(
        makeLoadedVertexBuffer(
            {
                {0, 4, offsetof(Vertex, mPosition), MappedGL<GLfloat>::enumerator},
                {1, 2, offsetof(Vertex, mUV),       MappedGL<GLint>::enumerator, ShaderAccess::Integer},
            },
            sizeof(Vertex),
            sizeof(gVerticesQuad),
            gVerticesQuad
        ));

    // Per-instance attributes
    mDrawContext.mVertexSpecification.mVertexBuffers.push_back(
        makeLoadedVertexBuffer<Instance>(
            {
                {2, 2, offsetof(Instance, mPosition),   MappedGL<GLint>::enumerator},
                {3, 4, offsetof(Instance, mTextureArea), MappedGL<GLint>::enumerator, ShaderAccess::Integer},
            },
            mSprites.begin(), mSprites.end()
        ));
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    //
    // Program
    //
    mDrawContext.mProgram = makeLinkedProgram({
                          {GL_VERTEX_SHADER, gVertexShader},
                          {GL_FRAGMENT_SHADER, gAnimationFragmentShader},
                      });

    // Use is required for setting, but we could replace with glProgramUniform instead!
    glUseProgram(mDrawContext.mProgram);
    glUniform1i(glGetUniformLocation(mDrawContext.mProgram, "spriteSampler"), 0);
}

void Engine::callbackWindowSize(int width, int height)
{
    glViewport(0, 0, width, height);
    mWindowSize.width() = width;
    mWindowSize.height() = height;

    GLint location = glGetUniformLocation(mDrawContext.mProgram, "in_BufferResolution");
    glProgramUniform2iv(mDrawContext.mProgram, location, 1, mWindowSize.data());
}

std::vector<Sprite> Engine::loadSheet(const std::string &aPath)
{
    json meta;
    std::ifstream{aPath} >> meta;

    {
        Image spriteSheet{pathFor(meta["file"])};
        Texture textureSheet{GL_TEXTURE_RECTANGLE};
        /// \todo somehow merge with loadSprite pre-exisiting function
        loadSpriteSheet(textureSheet, GL_TEXTURE0, spriteSheet, spriteSheet.dimension());
        mDrawContext.mTextures.push_back(std::move(textureSheet));
    }

    std::vector<Sprite> result;

    auto grid = meta["set"]["regularGrid"];
    math::Dimension2<int> dimension{grid["width"], grid["height"]};
    /// \todo Address correctly at the math library level
    math::Vec2<int> dimensionVec{grid["width"], grid["height"]};
    ///
    math::Vec2<int> tileOffset =
        //static_cast<math::Vec2<int>>(dimension) + math::Vec2<int>{grid["xBorder"], grid["yBorder"]};
        dimensionVec + math::Vec2<int>{grid["xBorder"], grid["yBorder"]};
    math::Vec2<int> startOffset = math::Vec2<int>{grid["xOffset"], grid["yOffset"]};
    const std::string prefix = meta["set"]["prefix"];

    for (int row : math::Range<int>{grid["yCount"]})
    {
        for (int column : math::Range<int>{grid["xCount"]})
        {
            std::ostringstream nameOss;
            nameOss << prefix << column << "_" << row;

            result.push_back({nameOss.str(),
                              startOffset + tileOffset.hadamard({column, row}),
                              dimension});
        }
    }

    return result;
}


void Engine::appendDraw(const Sprite & aSprite, Position aPosition)
{
    mSprites.emplace_back(aPosition, aSprite.mTextureArea);
}


void Engine::render()
{
    //
    // Stream vertex attributes
    //
    respecifyBuffer(mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    mSprites.data(),
                    getStoredSize(mSprites));

    //
    // Draw
    //
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesCount,
                          static_cast<GLsizei>(mSprites.size()));
}

} // namespace ad
