#pragma once
#include <glad/gl.h>
#include <iostream>
#include <cassert>

inline constexpr const char* glErrorName(GLenum errorCode) {
    switch (errorCode) {
        case GL_INVALID_ENUM:                  return "INVALID_ENUM";
        case GL_INVALID_VALUE:                 return "INVALID_VALUE";
        case GL_INVALID_OPERATION:             return "INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION";
    }
}

#if defined(NDEBUG)
#define GL_CHECK(stmt) stmt;
#else
#define GL_CHECK(stmt) stmt; \
    { GLenum err; \
    while((err = glGetError()) != GL_NO_ERROR){ \
        std::cout << "GL_ERROR [" << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << "] " << #stmt << " ! " << glErrorName(err) << '/' << err << '\n';  } }((void)0)
#endif