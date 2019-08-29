#pragma once

#include "Ring.h"
#include "shaders.h"

#include <handy/vector_utils.h>

#include <renderer/Drawing.h>
#include <renderer/Image.h>
#include <renderer/Shading.h>
#include <renderer/Texture.h>
#include <renderer/VertexSpecification.h>

#include <resource/PathProvider.h>

#include <math/Vector.h>

#include <glad/glad.h>

#include <random>


namespace ad
{

struct Vertex
{
    math::Vec4<GLfloat> mPosition;
    math::Vec2<GLfloat> mUV;
};

constexpr size_t gVerticesCount{4};

Vertex gVerticesQuad[gVerticesCount] = {
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

struct Entity
{
    Entity(DrawContext aDrawContext,
           std::function<void(Entity &, double)> aUpdater,
           std::function<void(const Entity &)> aDrawer) :
        mDrawContext(std::move(aDrawContext)),
        mUpdater(std::move(aUpdater)),
        mDrawer(std::move(aDrawer))
    {}

    void update(double aTime)
    {
        mUpdater(*this, aTime);
    }

    void draw() const
    {
        glBindVertexArray(mDrawContext.mVertexSpecification.mVertexArray);
        glUseProgram(mDrawContext.mProgram);
        mDrawer(*this);
    }

    DrawContext mDrawContext;
    std::function<void(Entity &, double)> mUpdater;
    std::function<void(const Entity &)> mDrawer;
};


struct Scene
{
    DrawContext mDrawContext;
    std::vector<Ring> mRings;
};

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
                    sizeof(gVerticesQuad),
                    gVerticesQuad
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

void drawRing(const Entity &aEntity)
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gVerticesCount);
}

void rotateRing(Entity &aEntity, double aTimeSeconds)
{
    constexpr int rotationsPerSec = 1;
    constexpr int frames = 8;
    glUniform1i(glGetUniformLocation(aEntity.mDrawContext.mProgram, "frame"),
                static_cast<int>(aTimeSeconds*rotationsPerSec*frames) % frames);
}


struct Randomizer
{
    Randomizer(int aMin, int aMax) :
        mMin(aMin),
        mMax(aMax),
        mUniform(aMin, aMax)
    {}

    int rand()
    {
        return mUniform(mEngine);
    }

    template <class T>
    T norm()
    {
        return mUniform(mEngine)/static_cast<T>(mMax);
    }

    int mMin;
    int mMax;
    std::default_random_engine mEngine{};
    std::uniform_int_distribution<int> mUniform;
};

Scene setupScene()
{
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Frame buffer clear color
    glClearColor(0.1f, 0.2f, 0.3f, 1.f);

    static const Image ring(pathFor("sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png"));

    //
    // Sub-parts
    //
    constexpr GLsizei width  = 347-3;
    constexpr GLsizei height = 303-3;

    Scene scene{animatedRing(ring, {width, height})};

    Randomizer pos(-128, 128);
    Randomizer speed(1, 4);

    constexpr int gRingCount = 200;
    for(int i=0; i!=gRingCount; ++i)
    {
        scene.mRings.push_back({{pos.norm<GLfloat>(), pos.norm<GLfloat>()},
                                 static_cast<GLfloat>(speed.rand())});
    }

    scene.mDrawContext.mVertexSpecification.mVertexBuffers.push_back(
        makeLoadedVertexBuffer(
            {
                {2, 2, offsetof(Ring, mPosition),       MappedGL<GLfloat>::enumerator},
                {3, 1, offsetof(Ring, mRotationsPerSec),MappedGL<GLfloat>::enumerator},
            },
            sizeof(Ring),
            getStoredSize(scene.mRings),
            scene.mRings.data()
    ));

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    return scene;
}

void updateScene(Scene &aScene, double aTimeSeconds)
{
    glUniform1f(glGetUniformLocation(aScene.mDrawContext.mProgram, "time"),
                static_cast<GLfloat>(aTimeSeconds));

    for(Ring & ring : aScene.mRings)
    {
        ring.mPosition.y() -= 0.02f;
        if (ring.mPosition.y() < -1.f)
        {
            ring.mPosition.y() = 1.f;
        }
    }

    respecifyBuffer(aScene.mDrawContext.mVertexSpecification.mVertexBuffers.back(),
                    aScene.mRings.data());
}

void renderScene(Scene &aScene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          gVerticesCount,
                          static_cast<GLsizei>(aScene.mRings.size()));
}

} // namespace ad
