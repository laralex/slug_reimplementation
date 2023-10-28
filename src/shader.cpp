#include "shader.hpp"
#include "util.hpp"

auto compileGlShader(GLenum shaderType, std::string_view filepath) -> std::variant<GLint, const char*> {
    static char INFO_LOG_BUFFER[512];

    GL_CHECK(GLint shader = glCreateShader(shaderType));
    auto shaderCode = readTextFile(filepath);
    assert(shaderCode.has_value());
    const char* shaderCodeRaw = shaderCode->c_str();
    GL_CHECK(glShaderSource(shader, 1, &shaderCodeRaw, nullptr));
    GL_CHECK(glCompileShader(shader));

    GLint success;
    GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if(success == 0)
    {
        GL_CHECK(glGetShaderInfoLog(shader, sizeof(INFO_LOG_BUFFER), nullptr, INFO_LOG_BUFFER));
        GL_CHECK(glDeleteShader(shader));
        return INFO_LOG_BUFFER; // NOTE: stored data is only valid until next call to this function
    }
    return shader;
}

auto linkGlProgram(GLint vertexShader, GLint fragmentShader) -> std::variant<GLint, const char*> {
    static char INFO_LOG_BUFFER[512];

    GLint program = glCreateProgram();
    GL_CHECK(glAttachShader(program, vertexShader));
    GL_CHECK(glAttachShader(program, fragmentShader));
    GL_CHECK(glLinkProgram(program));

    GLint success;
    GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &success));
    if(success == 0)
    {
        GL_CHECK(glGetProgramInfoLog(program, sizeof(INFO_LOG_BUFFER), nullptr, INFO_LOG_BUFFER));
        GL_CHECK(glDeleteProgram(program));
        return INFO_LOG_BUFFER; // NOTE: stored data is only valid until next call to this function
    }
    return program;
}