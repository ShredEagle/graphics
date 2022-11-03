#include "VertexSpecification.h"


namespace ad {
namespace graphics {


std::ostream & operator<<(std::ostream &aOut, const AttributeDescription & aDescription)
{
    return aOut << "Index " << aDescription.mIndex << " | "
                << "Dimension " << aDescription.mDimension << " | "
                << "Offset " << aDescription.mOffset << " | "
                << "Data " << aDescription.mDataType
                ;
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
        switch(attribute.mTypeInShader)
        {
            case ShaderParameter::Access::Float :
            {
                glVertexAttribPointer(attribute.mIndex,
                                      attribute.mDimension,
                                      attribute.mDataType,
                                      attribute.mNormalize,
                                      aStride,
                                      reinterpret_cast<const void*>(attribute.mOffset));
                break;
            }
            case ShaderParameter::Access::Integer :
            {
                glVertexAttribIPointer(attribute.mIndex,
                                       attribute.mDimension,
                                       attribute.mDataType,
                                       aStride,
                                       reinterpret_cast<const void*>(attribute.mOffset));
                break;
            }
        }

        if (aAttributeDivisor)
        {
            glVertexAttribDivisor(attribute.mIndex, aAttributeDivisor);
        }

        glEnableVertexAttribArray(attribute.mIndex);
    }
}

VertexBufferObject initVertexBuffer(const VertexArrayObject & aVertexArray,
                                    std::initializer_list<AttributeDescription> aAttributes,
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
                                    GLuint aAttributeDivisor)
{
    VertexBufferObject vbo = initVertexBuffer(aVertexArray, aAttributes, aStride, aAttributeDivisor);
    // The vertex buffer is still bound from initialization
    glBufferData(GL_ARRAY_BUFFER, aSize, aData, GL_STATIC_DRAW);
    return vbo;
}


} // namespace graphics
} // namespace ad
