#pragma once


#include <arte/gltf/Gltf.h>

#include <renderer/GL_Loader.h>


namespace ad {
namespace gltfviewer {


inline std::size_t getComponentSize(GLenum aComponentType)
{
    switch(aComponentType)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
        return sizeof(GLbyte);
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
        return sizeof(GLshort);
    case GL_UNSIGNED_INT:
        return sizeof(GLuint);
    case GL_FLOAT:
        return sizeof(GLfloat);
    }

    throw std::logic_error{std::string{"In "} + __func__ 
        + ", unhandled component type: " + std::to_string(aComponentType)};
}


struct VertexAttributeLayout
{
    GLint componentsPerAttribute;
    std::size_t occupiedAttributes{1}; // For matrices types, will be the number of columns.

    std::size_t totalComponents() const
    { return componentsPerAttribute * occupiedAttributes; }

    std::size_t byteSize(GLenum aComponentType) const
    {
        return totalComponents() * getComponentSize(aComponentType);
    }
};


using ElementType = arte::gltf::Accessor::ElementType;

const std::map<ElementType, VertexAttributeLayout> gElementTypeToLayout{
    {ElementType::Scalar, {1}},    
    {ElementType::Vec2,   {2}},    
    {ElementType::Vec3,   {3}},    
    {ElementType::Vec4,   {4}},    
    {ElementType::Mat2,   {2, 2}},    
    {ElementType::Mat3,   {3, 3}},    
    {ElementType::Mat4,   {4, 4}},    
};


} // namespace gltfviewer
} // namespace ad
