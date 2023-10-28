#pragma once
#include "prelude.hpp"
#include <variant>
#include <string_view>

auto compileGlShader(GLenum shaderType, std::string_view filepath) -> std::variant<GLint, const char*>;
auto linkGlProgram(GLint vertexShader, GLint fragmentShader) -> std::variant<GLint, const char*>;