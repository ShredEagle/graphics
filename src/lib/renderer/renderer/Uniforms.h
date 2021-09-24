#pragma once


#include "Shading.h"

#include "GL_Loader.h"

#include <math/Matrix.h>
#include <math/Vector.h>


namespace ad {


    inline void setUniform(Program & aProgram, const std::string & aNameInShader,
                           const math::Matrix<3, 3, GLfloat> & aMatrix)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniformMatrix3fv(aProgram, location, 1, false, aMatrix.data());
    }


    inline void setUniform(Program & aProgram, const std::string & aNameInShader,
                           const math::Matrix<4, 4, GLfloat> & aMatrix)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniformMatrix4fv(aProgram, location, 1, false, aMatrix.data());
    }


    template <class T_derived>
    inline void setUniform(Program & aProgram, const std::string & aNameInShader,
                           const math::Vector<T_derived, 2, GLfloat> & aVector)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniform2f(aProgram, location, aVector[0], aVector[1]);
    }


    template <class T_derived>
    inline void setUniform(Program & aProgram, const std::string & aNameInShader,
                           const math::Vector<T_derived, 3, GLfloat> & aVector)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniform3f(aProgram, location, aVector[0], aVector[1], aVector[2]);
    }


    template <class T_derived>
    inline void setUniform(Program & aProgram, const std::string & aNameInShader,
                           const math::Vector<T_derived, 4, GLfloat> & aVector)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniform4f(aProgram, location, aVector[0], aVector[1], aVector[2], aVector[3]);
    }


    inline void setUniformFloat(Program & aProgram, const std::string & aNameInShader,
                                GLfloat aFloat)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniform1f(aProgram, location, aFloat);
    }


    inline void setUniformInt(Program & aProgram, const std::string & aNameInShader,
                              GLint aInteger)
    {
        GLint location = glGetUniformLocation(aProgram, aNameInShader.c_str());
        glProgramUniform1i(aProgram, location, aInteger);
    }


} // namespace ad
