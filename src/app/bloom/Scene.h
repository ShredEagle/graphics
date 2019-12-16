#pragma once

#include "shaders.h"
#include "vertices.h"

#include <engine/Engine.h>
#include <engine/Timer.h>

#include <renderer/FrameBuffer.h>
#include <renderer/Shading.h>
#include <renderer/Texture.h>
#include <renderer/VertexSpecification.h>

#include <resource/PathProvider.h>

#include <gsl/span>

namespace ad {

template <class T_vertex>
VertexBufferObject loadVertexBuffer(const VertexArrayObject & aVertexArray,
                                    const std::initializer_list<AttributeDescription> & aAttributes,
                                    gsl::span<T_vertex> aVertices)
{
    glBindVertexArray(aVertexArray);
    return makeLoadedVertexBuffer(aAttributes,
                                  sizeof(T_vertex),
                                  sizeof(T_vertex)*aVertices.size(),
                                  aVertices.data());
}

struct ScreenQuad
{
    ScreenQuad();
    VertexSpecification mVertexSpec;
};

inline ScreenQuad::ScreenQuad()
{
    mVertexSpec.mVertexBuffers.push_back(
        loadVertexBuffer(mVertexSpec.mVertexArray,
                         gVertexScreenDescription,
                         gsl::span<VertexScreenQuad>{gVerticesScreen}));
}

struct CompleteFrameBuffer
{
    FrameBuffer frameBuffer;
    Texture colorTexture{GL_TEXTURE_2D};
};

template <std::size_t T_size>
std::array<CompleteFrameBuffer, T_size> initFrameBuffers(const Engine & aEngine)
{
    constexpr std::size_t gChainSize = T_size;
    std::array<CompleteFrameBuffer, gChainSize> buffers;

    for (auto & [frameBuffer, texture] : buffers)
    {
        bind(texture);
        const int width{aEngine.getFramebufferSize().width()};
        const int height{aEngine.getFramebufferSize().height()};
        allocateStorage(texture, GL_RGBA8, width, height);
        unbind(texture);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.mTarget, texture, 0);

        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            throw std::runtime_error("Incomplete framebuffer (" + std::to_string(__LINE__) + ")" );
        }
        // Textbook leak above, RAII this
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    return buffers;
}

struct FirstRender
{
    FirstRender();

    VertexArrayObject mVAO;
    VertexBufferObject mVertexData;
    Program mProgram;
};

inline FirstRender::FirstRender() :
    mVAO(),
    mVertexData(loadVertexBuffer(mVAO, gVertexSceneDescription, gsl::span<VertexScene>{gVerticesScene})),
    mProgram(makeLinkedProgram({
        {GL_VERTEX_SHADER, gInitialVertex},
        {GL_FRAGMENT_SHADER, gInitialFragment}}))
{}

struct Scene
{
    Scene(const char * argv[], const Engine * aEngine);
    void blur();
    void step(const Timer & aTimer);

    const Engine * mEngine;
    FirstRender mInitial;
    ScreenQuad mScreenQuad;
    //std::array<Texture, 2> mColors{GL_TEXTURE_RECTANGLE, GL_TEXTURE_RECTANGLE};
    std::array<CompleteFrameBuffer, 2> mBuffers;
    CompleteFrameBuffer mSceneFB;
    Texture mNeonTexture;
    Program mHBlurProgram;
    Program mVBlurProgram;
    Program mScreenProgram;
};

Scene::Scene(const char * argv[], const Engine * aEngine) :
    mEngine(aEngine),
    mInitial(),
    mBuffers(initFrameBuffers<2>(*aEngine)),
    mSceneFB(),
    mNeonTexture(GL_TEXTURE_2D),
    mHBlurProgram(makeLinkedProgram({
        {GL_VERTEX_SHADER, gScreenVertex},
        {GL_FRAGMENT_SHADER, gHBlurFragment}})),
    mVBlurProgram(makeLinkedProgram({
        {GL_VERTEX_SHADER, gScreenVertex},
        {GL_FRAGMENT_SHADER, gVBlurFragment}})),
    mScreenProgram(makeLinkedProgram({
        {GL_VERTEX_SHADER, gScreenVertex},
        {GL_FRAGMENT_SHADER, gScreenFragment}}))
{
    {
        auto & [frameBuffer, texture] = mSceneFB;
        bind(texture);
        const int width{aEngine->getFramebufferSize().width()};
        const int height{aEngine->getFramebufferSize().height()};
        allocateStorage(texture, GL_RGBA8, width, height);
        unbind(texture);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.mTarget, texture, 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mBuffers.at(0).colorTexture.mTarget, mBuffers.at(0).colorTexture, 0);

        unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);

        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            throw std::runtime_error("Incomplete framebuffer (" + std::to_string(__LINE__) + ")" );
        }
        // Textbook leak above, RAII this
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Texture
    {
        loadSprite(mNeonTexture, GL_TEXTURE0, Image(pathFor("st_outline.png")));
    }
}

inline void Scene::step(const Timer & aTimer)
{
    /***
     * Rendering the scene to a texture
     ***/
    // TODO first render should have depth buffer enabled, to be generic
    glBindFramebuffer(GL_FRAMEBUFFER, mSceneFB.frameBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO Why blur stop working if alpha is 0?
    static const std::array<float, 4> gClearColor = {0.f, 0.f, 0.f, 1.f};
    glClearBufferfv(GL_COLOR, 1, gClearColor.data());

    glBindVertexArray(mInitial.mVAO);
    glUseProgram(mInitial.mProgram);

    glActiveTexture(GL_TEXTURE0); // Already active and bound
    bind(mNeonTexture);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          4,
                          1);


    /* It is about rendering to screen quad now */
    glBindVertexArray(mScreenQuad.mVertexSpec.mVertexArray);

    /***
     * Filtering the rendered texture
     ***/
    // TODO should select the correct texture depending on the ping pong
    // or could be done only once if always using the same texture
    glProgramUniform1i(mHBlurProgram,
                       glGetUniformLocation(mHBlurProgram, "screenTexture"),
                       0);
    glProgramUniform1i(mVBlurProgram,
                       glGetUniformLocation(mVBlurProgram, "screenTexture"),
                       1);

    glActiveTexture(GL_TEXTURE0); // Already active, but good practice
    bind(mBuffers.at(0).colorTexture);

    glActiveTexture(GL_TEXTURE1);
    bind(mBuffers.at(1).colorTexture);

    for (int i = 0;
         i != (static_cast<int>(aTimer.mTime)%2 == 0 ? 0 : 2);
         ++i)
    {
        glUseProgram(i%2 ? mVBlurProgram : mHBlurProgram);

        glBindFramebuffer(GL_FRAMEBUFFER, mBuffers.at((i+1)%2).frameBuffer);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }


    /***
     * Drawing to the screen
     ***/
    // TODO Can we reuse the program, changing only the fragment shader?
    // Binds the default (window) framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(mScreenProgram);
    // TODO should select the correct texture depending on the ping pong
    // or could be done only once if always using the same texture
    glProgramUniform1i(mScreenProgram,
                       glGetUniformLocation(mScreenProgram, "sceneTexture"),
                       0);
    glProgramUniform1i(mScreenProgram,
                       glGetUniformLocation(mScreenProgram, "bloomTexture"),
                       1);

    glActiveTexture(GL_TEXTURE0);
    bind(mSceneFB.colorTexture);

    glActiveTexture(GL_TEXTURE1);
    bind(mBuffers.at(0).colorTexture);

    // TODO: replace by the more correct non-instanced draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


} // namespace ad
