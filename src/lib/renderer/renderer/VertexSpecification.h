#pragma once

/// \TODO move to a separate lib
#include <renderer/Handy.h>

#include <glad/glad.h>

#include <vector>

namespace ad
{

//using VertexArrayObject = ResourceGuard<GLuint>
//
//VertexArrayObject makeVAO(GLuint vaoIndex)
//{
//    return VertexArrayObject(vaoIndex, [](GLuint vaoIndex)(glDeleteVertexArrays, 1, &vaoIndex));
//}
//

class VertexArrayObject : public ResourceGuard<GLuint>
{
public:
    VertexArrayObject(GLuint aIndex) :
        ResourceGuard<GLuint>{aIndex, [](GLuint aIndex){glDeleteVertexArrays(1, &aIndex);}}
    {}
};


struct [[nodiscard]] VertexBufferObject
{
    // Disable copy
    VertexBufferObject(const VertexBufferObject &) = delete;
    VertexBufferObject & operator=(const VertexBufferObject &) = delete;

    // Movable
    VertexBufferObject(VertexBufferObject && aOther) :
        mVBOId{aOther.mVBOId},
        mAttributeId{aOther.mAttributeId}
    {
        aOther.mVBOId = 0;
    }
    //VertexBufferObject & operator=(VertexBufferObject && aOther)
    //{
    //    mVBOId = aOther.mVBOId;
    //    mAttributeId = aOther.mAttributeId;
    //    aOther.mVBOId = 0;
    //}

    VertexBufferObject(GLuint aVBOId, GLuint aAttributeId) :
        mVBOId{aVBOId},
        mAttributeId{aAttributeId}
    {}

    ~VertexBufferObject()
    {
        if (mVBOId) // avoid disabling if it was moved from
        {
            glDisableVertexAttribArray(mAttributeId); 
            glDeleteBuffers(1, &mVBOId); 
        }
    }

    GLuint mVBOId;
    GLuint mAttributeId;
};


struct [[nodiscard]] VertexSpecification
{
    VertexSpecification(VertexArrayObject aVertexArray,
                        std::vector<VertexBufferObject> aVertexBuffers) :
        mVertexArray{std::move(aVertexArray)},
        mVertexBuffers{std::move(aVertexBuffers)}
    {}

    VertexArrayObject  mVertexArray; 
    std::vector<VertexBufferObject> mVertexBuffers; 
};


template <class T_scalar>
struct MappedGl;

template <> struct MappedGl<GLfloat>
{ static const GLenum enumerator = GL_FLOAT; };

template <class T_element, int N_vertices, int N_attributeDimension>
VertexBufferObject makeAndLoadBuffer(GLuint aAttributeId,
                                     T_element (& data)[N_vertices][N_attributeDimension])
{
    GLuint vertexBufferId;
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glVertexAttribPointer(aAttributeId,
                          N_attributeDimension,
                          MappedGl<T_element>::enumerator,
                          GL_FALSE,
                          0,
                          0);
    glEnableVertexAttribArray(aAttributeId);

    return {vertexBufferId, aAttributeId};
}


} // namespace ad
