#pragma once

// Note Ad: I once thought this was related to a problem of cdecl vs stdcall on Windows
// see: https://github.com/Dav1dde/glad#glad-includes-windowsh-42
// But the actual solution was to specify GLAPIENTRY where pointer to GL functions are expected
//#ifdef _WIN32
//    #define APIENTRY __stdcall
//#endif

// Note Ad: Since the above defines are not needed, this file is currently only wrapping one include
// we can keep it around, might be usefull if someday the actual gl loader is abstracted away

#include <glad/glad.h>