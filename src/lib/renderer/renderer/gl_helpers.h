#pragma once

namespace ad {

    // Sadly, the glGen* symbols imported by the GL loader
    // are not compile time constants
//template<void(*F_glGenFun)(GLsizei, GLuint *)>
//GLint reserve()
//{
//    GLuint name;
//    (*F_glGenFun)(1, &name);
//    return name;
//}

inline GLuint reserve(void(*aGlGenFunction)(GLsizei, GLuint *))
{
    GLuint name;
    aGlGenFunction(1, &name);
    return name;
}

} // namespace ad
