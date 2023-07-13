#include "ShaderVertexAttribute.h"

#include <stdexcept>
#include <string>


namespace ad {
namespace graphics {


ShaderParameter::Access getResourceShaderAccess(GLenum aType)
{
    switch(aType)
    {
        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
        case GL_FLOAT_MAT4x2:
        case GL_FLOAT_MAT4x3:
        {
            return ShaderParameter::Access::Float;
        }

        case GL_DOUBLE:
        case GL_DOUBLE_VEC2:
        case GL_DOUBLE_VEC3:
        case GL_DOUBLE_VEC4:
        case GL_DOUBLE_MAT2:
        case GL_DOUBLE_MAT3:
        case GL_DOUBLE_MAT4:
        case GL_DOUBLE_MAT2x3:
        case GL_DOUBLE_MAT2x4:
        case GL_DOUBLE_MAT3x2:
        case GL_DOUBLE_MAT3x4:
        case GL_DOUBLE_MAT4x2:
        case GL_DOUBLE_MAT4x3:
        {
            throw std::logic_error{
                std::string{__func__} + ": Double parameters are not handled at the moment."};
        }

        case GL_INT:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:

        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_VEC2:
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
        {
            return ShaderParameter::Access::Integer;
        }

        default:
        {
            throw std::logic_error{
                std::string{__func__} + ": Unhandled enumerator with value: " 
                + std::to_string(aType) + "."};
        }
    }
}


AttributeDimension getResourceDimension(GLenum aType)
{
    switch(aType)
    {
        case GL_FLOAT:
        case GL_DOUBLE:
        case GL_INT:
        case GL_UNSIGNED_INT:
            return {1, 1};

        case GL_FLOAT_VEC2:
        case GL_DOUBLE_VEC2:
        case GL_INT_VEC2:
        case GL_UNSIGNED_INT_VEC2:
            return {2, 1};

        case GL_FLOAT_VEC3:
        case GL_DOUBLE_VEC3:
        case GL_INT_VEC3:
        case GL_UNSIGNED_INT_VEC3:
            return {3, 1};

        case GL_FLOAT_VEC4:
        case GL_DOUBLE_VEC4:
        case GL_INT_VEC4:
        case GL_UNSIGNED_INT_VEC4:
            return {4, 1};

        case GL_FLOAT_MAT2:
        case GL_DOUBLE_MAT2:
            return {2, 2};

        case GL_FLOAT_MAT3:
        case GL_DOUBLE_MAT3:
            return {3, 3};

        case GL_FLOAT_MAT4:
        case GL_DOUBLE_MAT4:
            return {4, 4};

        // In GL, matNxM has N column and M rows.
        // Following our convention (row major), 
        // the number of colums (i.e. the number of elements in a row) is the first dimension.
        case GL_FLOAT_MAT2x3:
        case GL_DOUBLE_MAT2x3:
            return {2, 3};

        case GL_FLOAT_MAT2x4:
        case GL_DOUBLE_MAT2x4:
            return {2, 4};

        case GL_FLOAT_MAT3x2:
        case GL_DOUBLE_MAT3x2:
            return {3, 2};

        case GL_FLOAT_MAT3x4:
        case GL_DOUBLE_MAT3x4:
            return {3, 4};

        case GL_FLOAT_MAT4x2:
        case GL_DOUBLE_MAT4x2:
            return {4, 2};

        case GL_FLOAT_MAT4x3:
            return {4, 3};

        default:
        {
            throw std::logic_error{
                std::string{__func__} + ": Unhandled enumerator with value: " 
                + std::to_string(aType) + "."};
        }
    }
}


GLenum getResourceComponentType(GLenum aType)
{
    switch(aType)
    {
        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
        case GL_FLOAT_MAT4x2:
        case GL_FLOAT_MAT4x3:
        {
            return GL_FLOAT;
        }

        case GL_DOUBLE:
        case GL_DOUBLE_VEC2:
        case GL_DOUBLE_VEC3:
        case GL_DOUBLE_VEC4:
        case GL_DOUBLE_MAT2:
        case GL_DOUBLE_MAT3:
        case GL_DOUBLE_MAT4:
        case GL_DOUBLE_MAT2x3:
        case GL_DOUBLE_MAT2x4:
        case GL_DOUBLE_MAT3x2:
        case GL_DOUBLE_MAT3x4:
        case GL_DOUBLE_MAT4x2:
        case GL_DOUBLE_MAT4x3:
        {
            return GL_DOUBLE;
        }

        case GL_INT:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:
        {
            return GL_INT;
        }

        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_VEC2:
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
        {
            return GL_UNSIGNED_INT;
        }

        case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
        {
            return GL_BOOL;
        }

        default:
        {
            throw std::logic_error{
                std::string{__func__} + ": Unhandled enumerator with value: " 
                + std::to_string(aType) + "."};
        }
    }
}


bool isResourceSamplerType(GLenum aType)
{
    switch(aType)
    {
        default:
        {
            return false;
        }

        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        {
            return true;
        }
    }
}

} // namespace graphics
} // namespace ad