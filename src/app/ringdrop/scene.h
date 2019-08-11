#pragma once

#include "shaders.h"

#include <renderer/Drawing.h>
#include <renderer/Image.h>
#include <renderer/Shading.h>
#include <renderer/Texture.h>
#include <renderer/VertexSpecification.h>

#include <math/Vector.h>

#include <glad/glad.h>


namespace ad
{

struct Vertex
{
    math::Vec4<GLfloat> mPosition;
    math::Vec2<GLfloat> mUV;
};

constexpr size_t gVerticesCount{4};

Vertex gVertices[gVerticesCount] = {
    {
        {-1.0f, -1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f},
    },
    {
        {-1.0f,  1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f},
    },
    {
        { 1.0f, -1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f},
    },
    {
        { 1.0f,  1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f},
    },
};

typedef DrawContext Scene;

//struct [[nodiscard]] Scene
//{
//    Scene(VertexSpecification aVertexSpecification,
//          Texture aTexture,
//          Program aProgram) :
//        mVertexSpecification{std::move(aVertexSpecification)},
//        mTexture{std::move(aTexture)},
//        mProgram{std::move(aProgram)}
//    {}
//
//    VertexSpecification mVertexSpecification;
//    Texture mTexture;
//    Program mProgram;
//};
//

DrawContext staticRing(const Image &aImage, const math::Dimension2<int> aFrame)
{
    DrawContext drawing = [&](){
        VertexSpecification specification;
        glBindVertexArray(specification.mVertexArray);

        specification.mVertexBuffers.emplace_back(
                makeLoadedVertexBuffer(
                    {
                        {0, 4, offsetof(Vertex, mPosition), MappedGL<GLfloat>::enumerator},
                        {1, 2, offsetof(Vertex, mUV),       MappedGL<GLfloat>::enumerator},
                    },
                    sizeof(Vertex),
                    sizeof(gVertices),
                    gVertices
                ));

        //
        // Program
        //
        Program program = makeLinkedProgram({
                                  {GL_VERTEX_SHADER, gVertexShader},
                                  {GL_FRAGMENT_SHADER, gFragmentShader},
                              });
        glUseProgram(program);

        glUniform1i(glGetUniformLocation(program, "spriteSampler"), 1);

        return DrawContext{std::move(specification), std::move(program)};
    }();

    //
    // Texture
    //
    {
        // First-sprite
        // Found by measuring in the image raster
        Image firstRing = aImage.crop({{3, 3}, aFrame});
        Texture texture{GL_TEXTURE_2D};
        loadSprite(texture, GL_TEXTURE1, firstRing);

        drawing.mTextures.push_back(std::move(texture));
    }

    return drawing;
}

DrawContext animatedRing(const Image &aImage, const math::Dimension2<int> aFrame)
{
    DrawContext drawing = [&](){
        VertexSpecification specification;
        glBindVertexArray(specification.mVertexArray);

        specification.mVertexBuffers.emplace_back(
                makeLoadedVertexBuffer(
                    {
                        {0, 4, offsetof(Vertex, mPosition), MappedGL<GLfloat>::enumerator},
                        {1, 2, offsetof(Vertex, mUV),       MappedGL<GLfloat>::enumerator},
                    },
                    sizeof(Vertex),
                    sizeof(gVertices),
                    gVertices
                ));

        //
        // Program
        //
        Program program = makeLinkedProgram({
                                  {GL_VERTEX_SHADER, gVertexShader},
                                  {GL_FRAGMENT_SHADER, gAnimationFragmentShader},
                              });
        glUseProgram(program);

        glUniform1i(glGetUniformLocation(program, "spriteSampler"), 2);

        return DrawContext{std::move(specification), std::move(program)};
    }();

    //
    // Texture
    //
    {
        // Complete animation
        std::vector<math::Vec2<int>> framePositions = {
                {3,    3},
                {353,  3},
                {703,  3},
                {1053, 3},
                {1403, 3},
                {1753, 3},
                {2103, 3},
                {2453, 3},
        };
        Image animationArray = aImage.prepareArray(framePositions, aFrame);

        // First-sprite
        // Found by measuring in the image raster
        Texture texture{GL_TEXTURE_2D_ARRAY};
        loadAnimation(texture, GL_TEXTURE2, animationArray, aFrame, framePositions.size());

        drawing.mTextures.push_back(std::move(texture));
    }

    return drawing;
}
Scene setupScene()
{
    //static const Image ring("/tmp/sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png");
    static const Image ring("d:/projects/sprites/sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png");

    //
    // Sub-parts
    //
    constexpr GLsizei width  = 347-3;
    constexpr GLsizei height = 303-3;

    DrawContext immobile = staticRing(ring, {width, height});
    DrawContext animated = animatedRing(ring, {width, height});

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return animated;
}

void updateScene(Scene &aScene, double aTimeSeconds)
{
    constexpr int rotationsPerSec = 1;
    constexpr int frames = 8;
    glUniform1i(glGetUniformLocation(aScene.mProgram, "frame"),
                static_cast<int>(aTimeSeconds*rotationsPerSec*frames) % frames);
    std::cerr << "time: " << aTimeSeconds << std::endl;
}

void renderScene(Scene &aScene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gVerticesCount);
}

} // namespace ad
