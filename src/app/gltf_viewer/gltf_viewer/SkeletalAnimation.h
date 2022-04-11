#pragma once

#include <arte/gltf/Gltf.h>

#include <math/Homogeneous.h>

#include <renderer/GL_Loader.h>
#include <renderer/Shading.h>
#include <renderer/UniformBuffer.h>

#include <map>
#include <span>


namespace ad {
namespace gltfviewer {


struct Joint
{
    math::AffineMatrix<4, GLfloat> worldTransform;
};


using JointRepository = std::map<arte::gltf::Index<arte::gltf::Node>, Joint>;


struct JointMatrixPalette
{
    JointMatrixPalette(std::size_t aMatrixCount);

    void update(std::span<math::AffineMatrix<4, GLfloat>> aMatrices);

    graphics::UniformBufferObject uniformBuffer;
};


struct Skeleton
{
    Skeleton(arte::Const_Owned<arte::gltf::Skin> aSkin);

    void updatePalette(const JointRepository & aJoints);

    std::vector<math::AffineMatrix<4, GLfloat>> inverseBindMatrices;
    std::vector<arte::gltf::Index<arte::gltf::Node>> joints; // Another copy from gltf structs...
    JointMatrixPalette matrixPalette;
};


} // namespace gltfviewer
} // namespace ad
