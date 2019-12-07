#pragma once

#include "shaders.h"
#include "vertices.h"

#include <engine/Engine.h>

#include <renderer/FrameBuffer.h>
#include <renderer/Shading.h>
#include <renderer/Texture.h>
#include <renderer/VertexSpecification.h>

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
    Texture colorTexture{GL_TEXTURE_RECTANGLE};
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

struct Scene
{
    Scene(const char * argv[], const Engine * aEngine);
    void blur();
    void step();

    const Engine * mEngine;
    FirstRender mInitial;
    ScreenQuad mScreenQuad;
    //std::array<Texture, 2> mColors{GL_TEXTURE_RECTANGLE, GL_TEXTURE_RECTANGLE};
    std::array<CompleteFrameBuffer, 2> mBuffers;
    Program mBlurProgram;
    Program mScreenProgram;
};

Scene::Scene(const char * argv[], const Engine * aEngine) :
    mEngine(aEngine),
    mInitial(),
    mBuffers(initFrameBuffers<2>(*aEngine))
    ,
    mBlurProgram(makeLinkedProgram({
        {GL_VERTEX_SHADER, gScreenVertex},
        {GL_FRAGMENT_SHADER, gBlurFragment}})),
    mScreenProgram(makeLinkedProgram({
        {GL_VERTEX_SHADER, gScreenVertex},
        {GL_FRAGMENT_SHADER, gScreenFragment}}))
{}

inline void Scene::step()
{
    /***
     * Rendering the scene to a texture
     ***/
    // TODO first render should have depth buffer enabled, to be generic
    glBindFramebuffer(GL_FRAMEBUFFER, mBuffers.at(0).frameBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(mInitial.mVAO);
    glUseProgram(mInitial.mProgram);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          4,
                          1);


    /* It is about rendering to screen quad now */
    glBindVertexArray(mScreenQuad.mVertexSpec.mVertexArray);

    /***
     * Filtering the rendered texture
     ***/
    glUseProgram(mBlurProgram);

    // TODO should select the correct texture depending on the ping pong
    // or could be done only once if always using the same texture
    glProgramUniform1i(mBlurProgram,
                       glGetUniformLocation(mBlurProgram, "screenTexture"),
                       0);

    for (int i = 0; i != 10; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mBuffers.at((i+1)%2).frameBuffer);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0); // Already active, but good practice
        bind(mBuffers.at(i%2).colorTexture);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }


    /***
     * Drawing to the screen
     ***/
    // TODO Can we reuse the program, changing only the fragment shader?
    // Binds the default (window) framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(mScreenProgram);
    // TODO should select the correct texture depending on the ping pong
    // or could be done only once if always using the same texture
    glProgramUniform1i(mScreenProgram,
                       glGetUniformLocation(mScreenProgram, "screenTexture"),
                       0);

    glActiveTexture(GL_TEXTURE0); // Already active, but good practice
    bind(mBuffers.at(1).colorTexture);

    // TODO: replace by the more correct non-instanced draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


} // namespace ad
