#pragma once


#include "AttributeDimension.h"
#include "GL_Loader.h"
#include "VertexSpecification.h"


namespace ad {
namespace graphics {


// Provide analysis for the shader program resource types, as queried with
// glGetProgramResourceiv().
// For the types in the enumeration, see OpenGL 4.6 core specification,
// table 7.3  p 116.


AttributeDimension getResourceDimension(GLenum aType);

ShaderParameter::Access getResourceShaderAccess(GLenum aType);

GLenum getResourceComponentType(GLenum aType);

bool isResourceSamplerType(GLenum aType);


} // namespace graphics
} // namespace ad