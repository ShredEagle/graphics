#pragma once


#include "GL_Loader.h"


namespace ad {
namespace graphics {


    // Sadly, the glGen* symbols imported by the GL loader
    // are not compile time constants
//template<void(*F_glGenFun)(GLsizei, GLuint *)>
//GLint reserve()
//{
//    GLuint name;
//    (*F_glGenFun)(1, &name);
//    return name;
//}


inline GLuint reserve(void(GLAPIENTRY * aGlGenFunction)(GLsizei, GLuint *))
{
    GLuint name;
    aGlGenFunction(1, &name);
    return name;
}


namespace detail 
{

template <class T_resource>
class NameBase
{
public:
    struct UnsafeTag{};

    /// \attention I intended this ctor to be private, but then there is a lot of friending to do.
    /// The provided name must be a valid name for T_resource.
    explicit NameBase(GLuint aResource, UnsafeTag) :
        mResource{aResource}
    {}

    /*implicit*/ NameBase(const T_resource & aResource) :
        mResource{aResource}
    {}

    /*implicit*/ operator const GLuint() const
    {
        return mResource;
    }

private:
    GLuint mResource;
};

} // namespace detail


/// \brief A non-owning copy of the resource, notably storing a copy of the GLuint "name" of the resource.
/// The advantage over the plain GLuint is it preserves some strong typing.
///
/// \attention Name is a misnomer, see for example Name<Texture> which also store the target.
/// Buffer name in the sense of the "name" returned by glGenBuffers (which is an unsigned integer)
template <class T_resource>
// TODO find a better class name, such a Value, or View (but it is not a view, since it does not get to the current value)
class Name : public detail::NameBase<T_resource>
{
public:
    using detail::NameBase<T_resource>::NameBase;
};


} // namespace graphics
} // namespace ad