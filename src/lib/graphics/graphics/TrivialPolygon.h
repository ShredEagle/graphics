#pragma once


#include "commons.h"

#include <renderer/Drawing.h>

#include <math/Homogeneous.h>


namespace ad {
namespace graphics {


class TrivialPolygon
{
public:
    struct PolygonPoint;

    /// \brief Initialize the rendering to show the rectangle {{0., 0.}, aRenderResolution}.
    TrivialPolygon(Size2<int> aRenderResolution);

    /// \brief Remove all shapes that were previously added.
    ///
    /// Should be called between each frame to implement animation.
    void clearPolygons();

    template <class T_inputIterator>
    void addVertices(T_inputIterator aFirst, T_inputIterator aLast);

    void addVertices(std::initializer_list<PolygonPoint> aPoints);
    void addVertices(std::vector<PolygonPoint> aPoints);

    /// \brief Render all shapes that were added since the last call to `clearShapes()`.
    void render();

    /// see: FoCG chapter 7 for this separation between several transformations.
    void setCameraTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);
    void setProjectionTransformation(const math::AffineMatrix<3, GLfloat> & aTransformation);

private:
    using Index = GLuint;

    static constexpr Index gRestartIndex = std::numeric_limits<Index>::max();

    DrawContext mDrawContext;
    std::vector<PolygonPoint> mVertexAttributes;
    IndexBufferObject mIbo;
    Index mNextIndex;
    std::vector<Index> mIndices;
};


struct TrivialPolygon::PolygonPoint
{
    math::Position<2, GLfloat> mPosition;
    Color mColor = math::sdr::gWhite;
    math::Matrix<3, 3, GLfloat> mMatrixTransform = math::Matrix<3, 3, GLfloat>::Identity();
};


template <class T_inputIterator>
void TrivialPolygon::addVertices(T_inputIterator aFirst, T_inputIterator aLast)
{
    std::move(aFirst, aLast, std::back_inserter(mVertexAttributes));

    for (auto count = std::distance(aFirst, aLast); count != 0; --count)
    {
        mIndices.push_back(mNextIndex++);
    }
    mIndices.push_back(gRestartIndex);
}


} // namespace graphics
} // namespace ad
