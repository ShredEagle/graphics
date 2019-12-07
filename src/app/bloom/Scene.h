#pragma once

#include "shaders.h"

#include <engine/Engine.h>

#include <renderer/FrameBuffer.h>
#include <renderer/Shading.h>
#include <renderer/Texture.h>
#include <renderer/VertexSpecification.h>

#include <gsl/span>

namespace ad {

struct VertexBloom
{
    Vec2<GLfloat> mPosition;
};

std::array<VertexBloom, 4> gVerticesBloom = {
    VertexBloom{
        {-1.0f, -1.0f},
    },
    VertexBloom{
        {-1.0f,  1.0f},
    },
    VertexBloom{
        { 1.0f, -1.0f},
    },
    VertexBloom{
        { 1.0f,  1.0f},
    },
};

constexpr std::initializer_list<AttributeDescription> gVertexDescription = {
    { 0, 2, offsetof(VertexBloom, mPosition), MappedGL<GLfloat>::enumerator},
};

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
    mVertexData(loadVertexBuffer(mVAO, gVertexDescription, gsl::span<VertexBloom>{gVerticesBloom})),
    mProgram(makeLinkedProgram({
        {GL_VERTEX_SHADER, gInitialVertex},
        {GL_FRAGMENT_SHADER, gInitialFragment}}))
{}

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
        const int width{aEngine.getWindowSize().width()};
        const int height{aEngine.getWindowSize().height()};
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
    FrameBuffer mInitialFB;
    //std::array<Texture, 2> mColors{GL_TEXTURE_RECTANGLE, GL_TEXTURE_RECTANGLE};
    std::array<CompleteFrameBuffer, 2> mBuffers;
    Program mScreenProgram;
};

Scene::Scene(const char * argv[], const Engine * aEngine) :
    mEngine(aEngine),
    mInitial(),
    mBuffers(initFrameBuffers<2>(*aEngine))
    ,
    mScreenProgram(makeLinkedProgram({
        {GL_VERTEX_SHADER, gScreenVertex},
        {GL_FRAGMENT_SHADER, gScreenFragment}}))
{}

inline void Scene::step()
{
    // TODO first render should have depth buffer enabled, to be generic
    glBindFramebuffer(GL_FRAMEBUFFER, mBuffers.at(0).frameBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(mInitial.mVAO);
    glUseProgram(mInitial.mProgram);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          4,
                          1);

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
    bind(mBuffers.at(0).colorTexture);

    // TODO: replace by the more correct non-instanced draw
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,
                          0,
                          4,
                          1);
}


} // namespace ad
