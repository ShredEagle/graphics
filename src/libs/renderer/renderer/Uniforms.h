#pragma once


#include "Shading.h"

#include "GL_Loader.h"

#include <math/Color.h>
#include <math/Matrix.h>
#include <math/Vector.h>


namespace ad {
namespace graphics {


    // Note: It is not obvious whether a const program should allow to change its parameters.
    // Yet, designing the API this way allow to make most render() member method constant.

    // Note: not taking a string_view, as we need a null-terminated c string from the name.
    /// \brief Overload taking the uniform name instead of its location.
    template <class T_value>
    inline void setUniform(const Program & aProgram, const std::string & aNameInShader,
                           const T_value & aValue)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        setUniform(aProgram, location, aValue);
    }
    

    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::Matrix<3, 3, GLfloat> & aMatrix)
    {
        glProgramUniformMatrix3fv(aProgram, aLocation, 1, false, aMatrix.data());
    }


    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::Matrix<4, 4, GLfloat> & aMatrix)
    {
        glProgramUniformMatrix4fv(aProgram, aLocation, 1, false, aMatrix.data());
    }

    template <class T_derived>
    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::Vector<T_derived, 2, GLint> & aVector)
    {
        glProgramUniform2i(aProgram, aLocation, aVector[0], aVector[1]);
    }


    template <class T_derived>
    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::Vector<T_derived, 3, GLint> & aVector)
    {
        glProgramUniform3i(aProgram, aLocation, aVector[0], aVector[1], aVector[2]);
    }


    template <class T_derived>
    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::Vector<T_derived, 2, GLfloat> & aVector)
    {
        glProgramUniform2f(aProgram, aLocation, aVector[0], aVector[1]);
    }


    template <class T_derived>
    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::Vector<T_derived, 3, GLfloat> & aVector)
    {
        glProgramUniform3f(aProgram, aLocation, aVector[0], aVector[1], aVector[2]);
    }


    template <class T_hdrNumber>
    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::hdr::Rgb<T_hdrNumber> & aColor)
    {
        glProgramUniform3f(aProgram, aLocation, (GLfloat)aColor[0], (GLfloat)aColor[1], (GLfloat)aColor[2]);
    }


    template <class T_derived>
    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::Vector<T_derived, 4, GLfloat> & aVector)
    {
        glProgramUniform4f(aProgram, aLocation, aVector[0], aVector[1], aVector[2], aVector[3]);
    }


    template <class T_hdrNumber>
    inline void setUniform(const Program & aProgram, GLint aLocation,
                           const math::hdr::Rgba<T_hdrNumber> & aColor)
    {
        glProgramUniform4f(aProgram, aLocation, (GLfloat)aColor[0], (GLfloat)aColor[1], (GLfloat)aColor[2], (GLfloat)aColor[3]);
    }


    inline void setUniform(const Program & aProgram, GLint aLocation,
                           GLfloat aFloat)
    {
        glProgramUniform1f(aProgram, aLocation, aFloat);
    }


    inline void setUniform(const Program & aProgram, GLint aLocation,
                           GLint aInteger)
    {
        glProgramUniform1i(aProgram, aLocation, aInteger);
    }


    inline void setUniform(const Program & aProgram, GLint aLocation,
                           GLuint aUnsignedInteger)
    {
        glProgramUniform1ui(aProgram, aLocation, aUnsignedInteger);
    }


    /// \brief  Set an array of scalar uniforms (i.e. each uniform in the array has dimension 1).
    inline void setUniformArray(const Program & aProgram, const std::string & aNameInShader,
                                std::span<const GLfloat> aFloats)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniform1fv(aProgram, location, (GLsizei)aFloats.size(), aFloats.data());
    }


    /// \brief  Set an array of scalar uniforms (i.e. each uniform in the array has dimension 1).
    inline void setUniformArray(const Program & aProgram, const std::string & aNameInShader,
                                std::span<const GLint> aIntegers)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniform1iv(aProgram, location, (GLsizei)aIntegers.size(), aIntegers.data());
    }


} // namespace graphics
} // namespace ad
