#include "GltfRendering.h"

#include "Logging.h"
#include "Shaders.h"

#include <renderer/Uniforms.h>

#include <math/Transformations.h>

#include <renderer/GL_Loader.h>


namespace ad {

using namespace arte;

namespace gltfviewer {


math::AffineMatrix<4, float> getLocalTransform(const arte::gltf::Node::TRS & aTRS)
{
    return 
        math::trans3d::scale(aTRS.scale.as<math::Size>())
        * aTRS.rotation.toRotationMatrix()
        * math::trans3d::translate(aTRS.translation)
        ;
}


math::AffineMatrix<4, float> getLocalTransform(const arte::gltf::Node & aNode)
{
    if(auto matrix = std::get_if<math::AffineMatrix<4, float>>(&aNode.transformation))
    {
        return *matrix;
    }
    else
    {
        return getLocalTransform(std::get<gltf::Node::TRS>(aNode.transformation));
    }
}


/// \brief A single (non-instanted) draw of the mesh primitive.
void render(const MeshPrimitive & aMeshPrimitive)
{
    graphics::bind_guard boundVao{aMeshPrimitive.vao};

    if (aMeshPrimitive.indices)
    {
        ADLOG(gDrawLogger, trace)
             ("Indexed rendering of {} vertices with mode {}.", aMeshPrimitive.count, aMeshPrimitive.drawMode);

        const Indices & indices = *aMeshPrimitive.indices;
        graphics::bind_guard boundIndexBuffer{indices.ibo};
        glDrawElements(aMeshPrimitive.drawMode, 
                       aMeshPrimitive.count,
                       indices.componentType,
                       reinterpret_cast<void *>(indices.byteOffset));
    }
    else
    {
        ADLOG(gDrawLogger, trace)
             ("Array rendering of {} vertices with mode {}.", aMeshPrimitive.count, aMeshPrimitive.drawMode);

        glDrawArrays(aMeshPrimitive.drawMode,  
                     0, // Start at the beginning of enable arrays, all byte offsets are aleady applied.
                     aMeshPrimitive.count);
    }
}


/// \brief Instantiated rendering of the mesh primitive.
void render(const MeshPrimitive & aMeshPrimitive, GLsizei aInstanceCount)
{
    graphics::bind_guard boundVao{aMeshPrimitive.vao};

    const auto & material = aMeshPrimitive.material;

    // Culling
    if (material.doubleSided) glDisable(GL_CULL_FACE);
    else glEnable(GL_CULL_FACE);

    // Alpha mode
    switch(material.alphaMode)
    {
    case gltf::Material::AlphaMode::Opaque:
        glDisable(GL_BLEND);
        break;
    case gltf::Material::AlphaMode::Blend:
        glEnable(GL_BLEND);
        break;
    case gltf::Material::AlphaMode::Mask:
        ADLOG(gDrawLogger, critical)("Not supported: mask alpha mode.");
        throw std::logic_error{"Mask alpha mode not implemented."};
    }

    glActiveTexture(GL_TEXTURE0 + Renderer::gTextureUnit);
    glBindTexture(GL_TEXTURE_2D, *material.baseColorTexture);

    if (aMeshPrimitive.indices)
    {
        ADLOG(gDrawLogger, trace)
             ("Indexed rendering of {} instance(s) of {} vertices with mode {}.",
              aMeshPrimitive.count, aInstanceCount, aMeshPrimitive.drawMode);

        const Indices & indices = *aMeshPrimitive.indices;
        graphics::bind_guard boundIndexBuffer{indices.ibo};
        glDrawElementsInstanced(
            aMeshPrimitive.drawMode, 
            aMeshPrimitive.count,
            indices.componentType,
            reinterpret_cast<void *>(indices.byteOffset),
            aInstanceCount);
    }
    else
    {
        ADLOG(gDrawLogger, trace)
             ("Instanced array rendering of {} instance(s) of {} vertices with mode {}.",
              aMeshPrimitive.count, aInstanceCount, aMeshPrimitive.drawMode);

        glDrawArraysInstanced(
            aMeshPrimitive.drawMode,  
            0, // Start at the beginning of enable arrays, all byte offsets are aleady applied.
            aMeshPrimitive.count,
            aInstanceCount);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}



Renderer::Renderer()
{
    initializePrograms();
}


void Renderer::render(const Mesh & aMesh) const
{
    std::shared_ptr<graphics::Program> program{mPrograms.at(aMesh.program)};
    graphics::bind_guard boundProgram{*program};

    setUniformInt(*program, "u_baseColorTex", gTextureUnit); 

    for (const auto & primitive : aMesh.primitives)
    {
        setUniform(*program, "u_baseColorFactor", primitive.material.baseColorFactor); 

        // If the vertex color are not provided for the primitive, the default value (black)
        // will be used in the shaders. It must be offset to white.
        auto vertexColorOffset = primitive.providesColor() ?
            math::hdr::Rgba_f{0.f, 0.f, 0.f, 0.f} : math::hdr::Rgba_f{1.f, 1.f, 1.f, 0.f};
        setUniform(*program, "u_vertexColorOffset", vertexColorOffset); 

        gltfviewer::render(primitive, aMesh.gpuInstances.size());
    }
}


void Renderer::bind(const Skeleton & aSkeleton)
{
    glBindBufferBase(GL_UNIFORM_BUFFER,
        gPaletteBlockBinding,
        aSkeleton.matrixPalette.uniformBuffer);
}


void Renderer::initializePrograms()
{
    auto setLights = [](graphics::Program & aProgram)
    {
        setUniform(aProgram, "u_light.position_world", math::Vec<4, GLfloat>{-100.f, 100.f, 1000.f, 1.f}); 
        setUniform(aProgram, "u_light.color", math::hdr::gWhite<GLfloat>);
        setUniformInt(aProgram, "u_light.specularExponent", 100);
        // Ideally, I suspect the sum should be 1
        setUniformFloat(aProgram, "u_light.diffuse", 0.3f);
        setUniformFloat(aProgram, "u_light.specular", 0.35f);
        setUniformFloat(aProgram, "u_light.ambient", 0.45f);
    };

    {
        auto phong = std::make_shared<graphics::Program>(
            graphics::makeLinkedProgram({
                {GL_VERTEX_SHADER,   gltfviewer::gPhongVertexShader},
                {GL_FRAGMENT_SHADER, gltfviewer::gPhongFragmentShader},
        }));
        setLights(*phong);
        mPrograms.emplace(GpuProgram::InstancedNoAnimation, std::move(phong));
    }

    {
        auto skinning = std::make_shared<graphics::Program>(
            graphics::makeLinkedProgram({
                {GL_VERTEX_SHADER,   gltfviewer::gSkeletalVertexShader},
                {GL_FRAGMENT_SHADER, gltfviewer::gPhongFragmentShader},
        }));
        if(auto paletteBlockId = glGetUniformBlockIndex(*skinning, "MatrixPalette");
           paletteBlockId != GL_INVALID_INDEX)
        {
            glUniformBlockBinding(*skinning, paletteBlockId, gPaletteBlockBinding);
        }
        else
        {
            throw std::logic_error{"Uniform block name could not be found."};
        }
        setLights(*skinning);
        mPrograms.emplace(GpuProgram::Skinning, std::move(skinning));
    }
}


void Renderer::setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    for (auto & [_key, program] : mPrograms)
    {
        setUniform(*program, "u_camera", aTransformation); 
    }
}


void Renderer::setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation)
{
    for (auto & [_key, program] : mPrograms)
    {
        setUniform(*program, "u_projection", aTransformation); 
    }
}


void Renderer::togglePolygonMode()
{
    switch(polygonMode)
    {
    case PolygonMode::Fill:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        polygonMode = PolygonMode::Line;
        break;
    case PolygonMode::Line:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        polygonMode = PolygonMode::Fill;
        break;
    }
}


} // namespace gltfviewer
} // namespace ad 