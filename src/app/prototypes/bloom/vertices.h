#pragma once

#include <renderer/commons.h>
#include <renderer/VertexSpecification.h>

#include <glad/glad.h>

#include <array>

namespace ad {

struct VertexScreenQuad
{
    Vec2<GLfloat> mPosition;
    Vec2<GLfloat> mUV;
};

std::array<VertexScreenQuad, 4> gVerticesScreen = {
    VertexScreenQuad{
        {-1.0f, -1.0f},
        {0.0f, 0.0f},
    },
    VertexScreenQuad{
        {-1.0f,  1.0f},
        {0.0f,  1.0f},
    },
    VertexScreenQuad{
        { 1.0f, -1.0f},
        { 1.0f, 0.0f},
    },
    VertexScreenQuad{
        { 1.0f,  1.0f},
        { 1.0f,  1.0f},
    },
};

constexpr std::initializer_list<AttributeDescription> gVertexScreenDescription = {
    { 0, 2, offsetof(VertexScreenQuad, mPosition), MappedGL<GLfloat>::enumerator},
    { 1, 2, offsetof(VertexScreenQuad, mUV),       MappedGL<GLfloat>::enumerator},
};

struct VertexScene
{
    Vec2<GLfloat> mPosition;
    Vec2<GLfloat> mUV;
};

std::array<VertexScene, 4> gVerticesScene = {
    VertexScene{
        {-1.0f, -1.0f},
        {0.0f, 0.0f},
    },
    VertexScene{
        {-1.0f,  1.0f},
        {0.0f, 1.0f},
    },
    VertexScene{
        { 1.0f, -1.0f},
        {1.0f, 0.0f},
    },
    VertexScene{
        { 1.0f,  1.0f},
        {1.0f, 1.0f},
    },
};

constexpr std::initializer_list<AttributeDescription> gVertexSceneDescription = {
    { 0, 2, offsetof(VertexScene, mPosition), MappedGL<GLfloat>::enumerator},
    { 1, 2, offsetof(VertexScreenQuad, mUV),  MappedGL<GLfloat>::enumerator},
};

} // namespace ad
