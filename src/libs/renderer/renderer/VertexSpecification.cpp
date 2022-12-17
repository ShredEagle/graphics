#include "VertexSpecification.h"


namespace ad {
namespace graphics {


std::ostream & operator<<(std::ostream & aOut, const AttributeDimension & aAttributeDimension)
{
    if (aAttributeDimension.mSecondDimension == 1)
    {
        return aOut << aAttributeDimension.mFirstDimension;
    }
    else
    {
        return aOut << "[" << aAttributeDimension[0] << ", " << aAttributeDimension[1] << "]";
    }
}

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

    auto enableAndDivisor = [aAttributeDivisor](GLuint aAttributeIndex)
    {
        if (aAttributeDivisor)
        {
            glVertexAttribDivisor(aAttributeIndex, aAttributeDivisor);
        }

        glEnableVertexAttribArray(aAttributeIndex);
    };

    for (const auto & attribute : aAttributes)
    {
        switch(attribute.mTypeInShader)
        {
            case ShaderParameter::Access::Float :
            {
                for(GLuint second = 0; second != attribute.mDimension[1]; ++second)
                {
                    glVertexAttribPointer(attribute.mIndex + second,
                                          attribute.mDimension[0],
                                          attribute.mDataType,
                                          attribute.mNormalize,
                                          aStride,
                                          reinterpret_cast<const void*>(attribute.mOffset 
                                            + second * attribute.sizeBytesFirstDimension()));
                    enableAndDivisor(attribute.mIndex + second);
                }
                break;
            }
            case ShaderParameter::Access::Integer :
            {
                for(GLuint second = 0; second != attribute.mDimension[1]; ++second)
                {
                    glVertexAttribIPointer(attribute.mIndex + second,
                                           attribute.mDimension[0],
                                           attribute.mDataType,
                                           aStride,
                                           reinterpret_cast<const void*>(attribute.mOffset
                                            + second * attribute.sizeBytesFirstDimension()));
                    enableAndDivisor(attribute.mIndex + second);
                }
                break;
            }
        }
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
