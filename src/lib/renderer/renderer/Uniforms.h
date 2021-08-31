#pragma once


#include "Shading.h"

#include "GL_Loader.h"

#include <math/Matrix.h>


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


} // namespace ad
