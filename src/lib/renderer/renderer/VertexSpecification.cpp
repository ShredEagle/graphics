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

VertexBufferObject makeLoadedVertexBuffer(std::initializer_list<AttributeDescription> aAttributes,
                                          GLsizei aStride,
                                          size_t aSize,
                                          const GLvoid * aData)
{
    /// \todo some static assert on the PODness of the element type

    VertexBufferObject vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, aSize, aData, GL_STATIC_DRAW);

    for (const auto & attribute : aAttributes)
    {
        switch(attribute.mTypeInShader)
        {
            case Attribute::Access::Float :
            {
                glVertexAttribPointer(attribute.mIndex,
                                      attribute.mDimension,
                                      attribute.mDataType,
                                      attribute.mNormalize,
                                      aStride,
                                      reinterpret_cast<const void*>(attribute.mOffset));
                break;
            }
            case Attribute::Access::Integer :
            {
                glVertexAttribIPointer(attribute.mIndex,
                                       attribute.mDimension,
                                       attribute.mDataType,
                                       aStride,
                                       reinterpret_cast<const void*>(attribute.mOffset));
                break;
            }
        }

        glEnableVertexAttribArray(attribute.mIndex);
    }

    return vbo;
}

} // namespace graphics
} // namespace ad
