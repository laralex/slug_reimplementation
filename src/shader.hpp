#pragma once
#include "prelude.hpp"
#include <variant>
#include <string_view>
#include <memory>

struct GlShaderHandle {
   GLint handle;
   ~GlShaderHandle() {
      if (handle > 0) { std::cout << "Shader deleter" << std::endl; GL_CHECK(glDeleteShader(handle)); }
   }
   GlShaderHandle(GLint handle = 0) : handle(handle) {};
   GlShaderHandle(GlShaderHandle&& rhs) : handle(rhs.handle) { rhs.handle = 0; };
   GlShaderHandle& operator=(GlShaderHandle&& rhs) {
      handle = rhs.handle; rhs.handle = 0; return *this;
   }
};

struct GlProgramHandle {
   GLint handle;
   ~GlProgramHandle() {
      if (handle > 0) { std::cout << "Program deleter" << std::endl; GL_CHECK(glDeleteProgram(handle)); }
   }
   GlProgramHandle(GLint handle = 0) : handle(handle) {};
   GlProgramHandle(GlProgramHandle&& rhs) : handle(rhs.handle) { rhs.handle = 0; };
   GlProgramHandle& operator=(GlProgramHandle&& rhs) {
      handle = rhs.handle; rhs.handle = 0; return *this;
   };
};

auto compileGlShader(GLenum shaderType, std::string_view filepath) -> std::variant<GlShaderHandle, const char*>;
auto linkGlProgram(GlShaderHandle&& vertexShader, GlShaderHandle&& fragmentShader) -> std::variant<GlProgramHandle, const char*>;