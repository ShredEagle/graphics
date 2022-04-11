#pragma once


#include "Mesh.h"
#include "SkeletalAnimation.h"

#include <arte/gltf/Gltf.h>

#include <renderer/Shading.h>


namespace ad {
namespace gltfviewer {


math::AffineMatrix<4, float> getLocalTransform(const arte::gltf::Node::TRS & aTRS);
math::AffineMatrix<4, float> getLocalTransform(const arte::gltf::Node & aNode);


enum class GpuProgram
{
    InstancedNoAnimation,
    Skinning,
};


class Renderer
{
public:
    Renderer();

    void setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation);

    void togglePolygonMode();

    void bind(const Skeleton & aSkeleton) const;

    void render(const Mesh & aMesh) const;
    void render(const Mesh & aMesh, const Skeleton & aSkeleton) const;

    static constexpr GLsizei gTextureUnit{0};
    static constexpr GLuint gPaletteBlockBinding{3};

private:
    enum class PolygonMode
    {
        Fill,
        Line,
    };

    void initializePrograms();
    template <class ... VT_extraParams>
    void renderImpl(const Mesh & aMesh, graphics::Program & aProgram, VT_extraParams ... aExtraParams) const;

    std::map<GpuProgram, std::shared_ptr<graphics::Program>> mPrograms;
    PolygonMode polygonMode{PolygonMode::Fill};
};

} // namespace gltfviewer
} // namespace ad
