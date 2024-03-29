#include "VertexSpecification.h"


namespace ad {
namespace graphics {

namespace {

    void enableAndDivisor(GLuint aAttributeIndex, GLuint aAttributeDivisor)
    {
        if (aAttributeDivisor)
        {
            glVertexAttribDivisor(aAttributeIndex, aAttributeDivisor);
        }
        glEnableVertexAttribArray(aAttributeIndex);
    };

} // anonymous


std::ostream & operator<<(std::ostream &aOut, const AttributeFormat & aDescription)
{
    return aOut << "Index " << aDescription.mIndex << " | "
                << "Dimension " << aDescription.mDimension << " | "
                << "Offset " << aDescription.mOffset << " | "
                << "Data " << aDescription.mComponentType
                ;
}


void attachBoundVertexBuffer(AttributeFormat aAttribute,
                             GLsizei aStride,
                             GLuint aAttributeDivisor)
{
    switch(aAttribute.mTypeInShader)
    {
        case ShaderParameter::Access::Float :
        {
            for(GLuint second = 0; second != aAttribute.mDimension[1]; ++second)
            {
                glVertexAttribPointer(aAttribute.mIndex + second,
                                      aAttribute.mDimension[0],
                                      aAttribute.mComponentType,
                                      aAttribute.mNormalize,
                                      aStride,
                                      reinterpret_cast<const void*>(
                                        aAttribute.mOffset 
                                        + second * aAttribute.sizeBytesFirstDimension()));
                enableAndDivisor(aAttribute.mIndex + second, aAttributeDivisor);
            }
            break;
        }
        case ShaderParameter::Access::Integer :
        {
            for(GLuint second = 0; second != aAttribute.mDimension[1]; ++second)
            {
                glVertexAttribIPointer(aAttribute.mIndex + second,
                                       aAttribute.mDimension[0],
                                       aAttribute.mComponentType,
                                       aStride,
                                       reinterpret_cast<const void*>(
                                            aAttribute.mOffset 
                                            + second * aAttribute.sizeBytesFirstDimension()));
                enableAndDivisor(aAttribute.mIndex + second, aAttributeDivisor);
            }
            break;
        }
    }
}


void attachVertexBuffer(const VertexBufferObject & aVertexBuffer,
                        const VertexArrayObject & aVertexArray,
                        AttributeDescriptionList aAttributes,
                        GLsizei aStride,
                        GLuint aAttributeDivisor)
{
    // TODO some static assertions on the PODness of the element type
    glBindVertexArray(aVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, aVertexBuffer);

    for (const auto & attribute : aAttributes)
    {
        attachBoundVertexBuffer(attribute, aStride, aAttributeDivisor);
    }
}

VertexBufferObject initVertexBuffer(const VertexArrayObject & aVertexArray,
                                    std::initializer_list<AttributeFormat> aAttributes,
                                    GLsizei aStride,
                                    GLuint aAttributeDivisor)
{
    VertexBufferObject vbo;
    attachVertexBuffer(vbo, aVertexArray, aAttributes, aStride, aAttributeDivisor);
    return vbo;
}

VertexBufferObject loadVertexBuffer(const VertexArrayObject & aVertexArray,
                                    AttributeDescriptionList aAttributes,
                                    GLsizei aStride,
                                    size_t aSize,
                                    const GLvoid * aData,
                                    BufferHint aHint,
                                    GLuint aAttributeDivisor)
{
    VertexBufferObject vbo = initVertexBuffer(aVertexArray, aAttributes, aStride, aAttributeDivisor);
    // The vertex buffer is still bound from initialization
    glBufferData(GL_ARRAY_BUFFER, aSize, aData, getGLBufferHint(aHint));
    return vbo;
}


} // namespace graphics
} // namespace ad
