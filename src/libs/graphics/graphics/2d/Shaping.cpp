#include "Shaping.h"

#include "Shaping-shaders.h"

#include <math/Constants.h>


namespace ad {
namespace graphics {


CameraProjection::CameraProjection()
{ 
    const std::array<math::Matrix<4, 4, GLfloat>, 2> identities{
        math::Matrix<4, 4, GLfloat>::Identity(),
        math::Matrix<4, 4, GLfloat>::Identity(),
    };

    bind_guard bound{mUniformBuffer};
    glBufferData(GL_UNIFORM_BUFFER, sizeof(identities), identities.data(), GL_DYNAMIC_DRAW);
}


void CameraProjection::setCameraTransformation(const math::AffineMatrix<4, GLfloat> & aTransformation)
{
    bind_guard bound{mUniformBuffer};
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(aTransformation),
                    aTransformation.data());
}


void CameraProjection::setProjectionTransformation(const math::Matrix<4, 4, GLfloat> & aTransformation)
{
    bind_guard bound{mUniformBuffer};
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(aTransformation), sizeof(aTransformation),
                    aTransformation.data());
}


namespace r2d {


namespace {

    template <std::size_t N_vertice>
    constexpr std::array<Vertex, N_vertice> getCircleVertices(GLfloat radius)
    {
        std::array<Vertex, N_vertice> vertices{
            Vertex{ .mPosition_local = {radius, 0.f} },
        };

        for(std::size_t n = 1; n < N_vertice; ++n)
        {
            GLfloat const t = 2 * math::pi<GLfloat> * n / N_vertice;
            vertices[n] = Vertex{
                .mPosition_local = {cos(t)*radius, sin(t)*radius}
            };
        }

        return vertices;
    }

    VertexBufferObject makeCircleVBO(VertexArrayObject & aVAO)
    {
        auto vertices = getCircleVertices<Shaping::Circle::gVerticesCount>(1.);
        return loadVertexBuffer(aVAO,
                                Vertex::gAttributeDescription,
                                std::span{vertices});
    }

} // anonymous namespace


ShapeSet::ShapeSet() :
    mCirclesVertexData{makeCircleVBO(mCirclesVAO)},
    mCirclesInstanceData{
        initVertexBuffer<Instance>(
            mCirclesVAO,
            Instance::gAttributeDescription,
            1)}
{}


void ShapeSet::resetCircles(std::span<Shaping::Circle> aCircles)
{
    respecifyBuffer(mCirclesInstanceData, aCircles);
    mCirclesCount = aCircles.size();
}


Shaping::Shaping() :
    mProgram{
        makeLinkedProgram({
            {GL_VERTEX_SHADER, {ShapingShaders::gVertexShader, "ShapingShaders::gVertexShader"}},
            {GL_FRAGMENT_SHADER, {ShapingShaders::gFragmentShader, "ShapingShaders::gFragmentShader"}},
        })},
    mViewingBlockIndex{glGetUniformBlockIndex(mProgram, "ViewingBlock")}
{}


void Shaping::render(const ShapeSet & aShapes, const CameraProjection & aCameraProjection) const
{
    aCameraProjection.bind();
    glUniformBlockBinding(mProgram, mViewingBlockIndex, CameraProjection::gBinding);
    
    bind(aShapes.mCirclesVAO);
    use(mProgram);

    glDrawArraysInstanced(GL_TRIANGLE_FAN,
                          0,
                          Circle::gVerticesCount,
                          aShapes.mCirclesCount);
}


} // namespace r2d
} // namespace graphics
} // namespace ad
