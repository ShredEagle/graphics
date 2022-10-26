#pragma once


#include <math/Homogeneous.h>
#include <math/Vector.h>

#include <renderer/UniformBuffer.h>
#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

#include <span>


namespace ad {
namespace graphics {


class CameraProjection
{
public:
    static const GLuint gBinding{1};

    //CameraProjection();

    void setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation);

    void bind() const
    { glBindBufferBase(GL_UNIFORM_BUFFER, gBinding, mUniformBuffer); }

private:
    // This is used as a default member initliazer, to avoid implementing a default ctor.
    // Not having a default ctor allows to rely on the "rule of zero".
    static UniformBufferObject IdentityBuffer();

    UniformBufferObject mUniformBuffer{IdentityBuffer()};
};


namespace r2d {


struct Vertex
{
    math::Position<2, GLfloat> mPosition_local{0.f, 0.f};

    static constexpr AttributeDescriptionList gAttributeDescription = {
        {0, 2, 0, MappedGL<GLfloat>::enumerator} 
    };
};

struct Instance
{
    // Position is 3D because:
    //* It allows to handle Z-ordering in 2D (with orthogonal projection)
    //* It allows to draw debugging 2D billboard shapes in a 3D world space (with perspective transform)
    math::Position<3, GLfloat> mPosition_world;
    math::Size<2, GLfloat> mSize_world;

    static constexpr AttributeDescriptionList gAttributeDescription = {
        {1, 3, 0,                        MappedGL<GLfloat>::enumerator},
        {2, 2, sizeof(mPosition_world),  MappedGL<GLfloat>::enumerator},
    };
};


class ShapeSet;


class Shaping
{
public:
    Shaping();

    struct Circle : public Instance
    {
        /*implicit*/ Circle(math::Position<3, GLfloat> aPosition_world, GLfloat aRadius = 1.f) :
            Instance{aPosition_world, math::Size<2, GLfloat>{aRadius, aRadius}}
        {}

        static const std::size_t gVerticesCount = 64;
    };

    void render(const ShapeSet & aShapes, const CameraProjection & aCameraProjection) const;

private:
    Program mProgram;
    GLuint mViewingBlockIndex;
};


class ShapeSet
{
    friend class Shaping;

public:
    ShapeSet();

    void resetCircles(std::span<Shaping::Circle> aCircles);

private:
    VertexArrayObject mCirclesVAO;
    // do NOT use const data members, they prevent move semantic, and we love move semantic.
    /*const*/ VertexBufferObject mCirclesVertexData;
    VertexBufferObject mCirclesInstanceData;
    GLsizei mCirclesCount{0};
};


} // namespace r2d
} // namespace graphics
} // namespace ad
