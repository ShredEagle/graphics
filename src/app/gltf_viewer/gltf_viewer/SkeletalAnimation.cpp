#include "SkeletalAnimation.h"

#include "LoadBuffer.h"
#include "Shaders.h"


namespace ad {
namespace gltfviewer {

using Matrix = math::AffineMatrix<4, GLfloat>;

JointMatrixPalette::JointMatrixPalette(std::size_t aMatrixCount)
{
    graphics::bind_guard bound{uniformBuffer};
    glBufferData(GL_UNIFORM_BUFFER,
        sizeof(Matrix) * aMatrixCount, 
        nullptr,
        GL_DYNAMIC_DRAW);
}


void JointMatrixPalette::update(std::span<Matrix> aMatrices)
{
    graphics::bind_guard bound{uniformBuffer};
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix) * aMatrices.size(), aMatrices.data());
}


Skeleton::Skeleton(arte::Const_Owned<arte::gltf::Skin> aSkin) :
    joints{aSkin->joints},
    matrixPalette{aSkin->joints.size()}
{
    // The palette size upper limit is hardcoded in the vertex shader
    assert(joints.size() <= 64);

    // TODO Ad 2022/04/01 Get rid of the useless copy.
    std::vector<std::byte> raw = loadBufferData(aSkin.get(&arte::gltf::Skin::inverseBindMatrices));
    auto matrix = reinterpret_cast<Matrix*>(raw.data());
    std::copy(matrix, matrix + aSkin->joints.size(), std::back_inserter(inverseBindMatrices));
};


void Skeleton::updatePalette(const JointRepository & aJoints)
{
    std::vector<Matrix> paletteData;
    paletteData.reserve(joints.size());

    for (std::size_t jointId = 0; jointId != joints.size(); ++jointId)
    {
        paletteData.push_back(inverseBindMatrices.at(jointId) * aJoints.at(joints.at(jointId)).worldTransform); 
    }
    matrixPalette.update(paletteData);
}


} // namespace gltfviewer
} // namespace ad
