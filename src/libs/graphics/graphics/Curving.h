#pragma once


#include <renderer/Drawing.h>
#include <renderer/VertexSpecification.h>

#include <math/Homogeneous.h>
#include <math/Curves/Bezier.h>


namespace ad {
namespace graphics {


class Curving
{
public:
    struct Instance
    {
        math::Bezier<4, 3, GLfloat> bezier; // cubic beziers in space of dimension 3
        GLfloat startHalfWidth;
        GLfloat endHalfWidth{ startHalfWidth };
        math::AffineMatrix<4, GLfloat> modelTransform{ math::AffineMatrix<4, GLfloat>::Identity() };

        using Position_t = decltype(bezier)::Position_t;
    };


    /// \param aCurveSubdivisions is the number of line segments used to approximate the curve.
    explicit Curving(GLsizei aCurveSubdivisions,
                     math::AffineMatrix<4, GLfloat> aProjectionTransformation = math::AffineMatrix<4, GLfloat>::Identity());

    void render(std::span<const Instance> aInstances) const;

    void setColor(math::hdr::Rgba_f aColor);

    void setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation);

private:
    GLsizei mVertexCount; 
    VertexArrayObject mVertexArray;
    VertexBufferObject mVertexBuffer;
    VertexBufferObject mInstanceBuffer;
    Program mGpuProgram;
};


} // namespace graphics
} // namespace ad
