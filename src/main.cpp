#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define TTF_FONT_PARSER_IMPLEMENTATION 1
#include <ttfparser.h>

#include "prelude.hpp"
#include "util.hpp"
#include "window.hpp"
#include "shader.hpp"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <variant>
#include <memory>


struct VertexData {
    float x, y;
    float u, v;
};

static void errorCallback(int error, const char* description) {
    (void)error;
    std::cerr << "Error: " << description << '\n';
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
 
static void onFontParsed(void* args, void* _font_data, int error) {
    if (error) {
		*(uint8_t*)args = error;
		std::cout << "Unable to parse font";
        return;
	}
	TTFFontParser::FontData* fontData = (TTFFontParser::FontData*)_font_data;
    printf("Font: %s %s parsed\n", fontData->font_names.begin()->font_family.c_str(), fontData->font_names.begin()->font_style.c_str());
    printf("Number of glyphs: %d\n", uint32_t(fontData->glyphs.size()));

    //step through glyph geometry
    {
        size_t numGlyphsToPrint = 30;
        for (const auto& glyphIterator : fontData->glyphs) {
            if (numGlyphsToPrint-- <= 0) {
                break;
            }
            uint32_t numCurves = 0, numLines = 0;
            for (const auto& pathList : glyphIterator.second.path_list) {
                for (const auto& geometry : pathList.geometry) {
                    if (geometry.is_curve)
                        numCurves++;
                    else
                        numLines++;
                }
            }
            std::wcout << "glyph " << glyphIterator.first << ' ' << static_cast<wchar_t>(glyphIterator.first) << " : n_curves=" << numCurves << " n_lines=" << numLines << '\n';
        }
    }

    *(uint8_t*)args = 1;
}

struct MeshData {
    std::vector<VertexData> vertexAttributes;
    std::vector<uint16_t> faceIndices;
};

MeshData convertGlyphToMesh(TTFFontParser::Glyph const& glyph) {
    //glyph.num_contours;
    //glyph.pathList;
    //glyph.bounding_box[4];
    
    float cx = glyph.glyph_center.x;
    float cy = glyph.glyph_center.y;
    float left = (glyph.bounding_box[0] - cx);
    float bottom = (glyph.bounding_box[1] - cy);
    float right = (glyph.bounding_box[2] - cx);
    float top = (glyph.bounding_box[3] - cy);

    MeshData out;
    
    size_t vidx = 0;
    out.vertexAttributes.resize(4);
    out.vertexAttributes[vidx] = {right, top, 1.0f, 1.0f }; ++vidx;
    out.vertexAttributes[vidx] = {left, top, 0.0f, 1.0f }; ++vidx;
    out.vertexAttributes[vidx] = {left, bottom, 0.0f, 0.0f}; ++vidx;
    out.vertexAttributes[vidx] = {right, bottom, 1.0f, 0.0f}; ++vidx;

    for(size_t i = 0; i < vidx; ++i) {
        std::cout << "x: " << out.vertexAttributes[i].x << " y: " << out.vertexAttributes[i].y << '\n';
    }
    size_t fidx = 0;
    out.faceIndices.resize(2 * 3);
    out.faceIndices[3*fidx] = 0;
    out.faceIndices[3*fidx + 1] = 1;
    out.faceIndices[3*fidx + 2] = 2;
    ++fidx;
    out.faceIndices[3*fidx] = 0;
    out.faceIndices[3*fidx + 1] = 2;
    out.faceIndices[3*fidx + 2] = 3;
    ++fidx;
    for(size_t i = 0; i < fidx; ++i) {
        std::cout << out.faceIndices[3*i] << ' ' << out.faceIndices[3*i + 1] << ' ' << out.faceIndices[3*i + 2] << '\n';
    }

    return out;
}


auto makeGlProgram(const char* vertexShaderFilepath, const char* fragmentShaderFilepath) -> GlProgramHandle {
    auto vertexShader = GlShaderHandle{};
    if (auto maybeShader = compileGlShader(GL_VERTEX_SHADER, "./assets/shaders/shader.vert"); isOk(maybeShader)) {
        vertexShader = unwrap(std::move(maybeShader));
    } else {
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << unwrapErr(std::move(maybeShader)) << std::endl;
        exit(EXIT_FAILURE);
    }

    auto fragmentShader = GlShaderHandle{};
    if (auto maybeShader = compileGlShader(GL_FRAGMENT_SHADER, "./assets/shaders/shader.frag"); isOk(maybeShader)) {
        fragmentShader = unwrap(std::move(maybeShader));
    } else {
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << unwrapErr(std::move(maybeShader)) << std::endl;
        exit(EXIT_FAILURE);
    }

    auto program = GlProgramHandle{};
    if (auto maybeProgram = linkGlProgram(std::move(vertexShader), std::move(fragmentShader)); isOk(maybeProgram)) {
        program = unwrap(std::move(maybeProgram));
    } else {
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << unwrapErr(std::move(maybeProgram)) << std::endl;
        exit(EXIT_FAILURE);
    }
    return program;
}

int main(void)
{
    //std::cout << "DEBUG " << DEBUG << " NDEBUG " << NDEBUG;
	TTFFontParser::FontData fontData;
    {
        uint8_t condition;
	    int8_t error = TTFFontParser::parse_file("./assets/fonts/HHSamuel.ttf", &fontData, onFontParsed, &condition);
        if (error) {
            exit(EXIT_FAILURE);
        }

    }

    uint32_t glyphCode = 321;
    std::wcout << L"Parsing glyph: " <<(wchar_t)glyphCode << '\n';
    MeshData glyphMesh = convertGlyphToMesh(fontData.glyphs[glyphCode]);

    GLuint vao, vbo, ebo;
    GLint mvpLocation, vposLocation, vuvLocation;
    GLint success;
 
    GlfwWindowHandle window;
    if (auto maybeWindow = initGlfw(); isOk(maybeWindow)) {
        window = unwrap(std::move(maybeWindow));
    } else {
        std::cout << unwrapErr(std::move(maybeWindow));
    }
    glfwSetKeyCallback(window.window, keyCallback);
    glfwSetErrorCallback(errorCallback);

    int versionGl = gladLoadGL(glfwGetProcAddress);
    std::cout << "Loaded OpenGL " << GLAD_VERSION_MAJOR(versionGl) << '.' << GLAD_VERSION_MINOR(versionGl) << '\n';

    auto imguiContext = initImgui(window.window);
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // NOTE: OpenGL error checks have been omitted for brevity
    GL_CHECK(glGenVertexArrays(1, &vao));
    GL_CHECK(glBindVertexArray(vao));
    GL_CHECK(glGenBuffers(1, &vbo));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(glyphMesh.vertexAttributes[0]) * glyphMesh.vertexAttributes.size(), glyphMesh.vertexAttributes.data(), GL_STATIC_DRAW));
    GL_CHECK(glGenBuffers(1, &ebo));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glyphMesh.faceIndices[0]) * glyphMesh.faceIndices.size(), glyphMesh.faceIndices.data(), GL_STATIC_DRAW));

    GlProgramHandle program = makeGlProgram("./assets/shaders/shader.vert", "./asserts/shaders/shader.frag");

    GL_CHECK(mvpLocation = glGetUniformLocation(program.handle, "MVP"));
    GL_CHECK(vposLocation = glGetAttribLocation(program.handle, "vPos"));
    GL_CHECK(vuvLocation = glGetAttribLocation(program.handle, "vUv"));
 
    GLuint vposLocationUnsigned = static_cast<GLuint>(vposLocation);
    GLuint vuvLocationUnsigned = static_cast<GLuint>(vuvLocation);
    GL_CHECK(glEnableVertexAttribArray(vposLocationUnsigned));
    GL_CHECK(glVertexAttribPointer(vposLocationUnsigned, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), nullptr));
    GL_CHECK(glEnableVertexAttribArray(vuvLocationUnsigned));
    GL_CHECK(glVertexAttribPointer(vuvLocationUnsigned, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), reinterpret_cast<const void*>(sizeof(float) * 2)));
 
    glm::vec4 clear_color (0.0f, 0.0f, 0.0f, 1.00f);
    static float modelRotationRadians = 0.0f, modelScale = 1.0f;
    static glm::vec3 modelTranslation = glm::vec3(0.f, 0.f, 500.f);
    static float frustumWidthHeight[2] = { 10.f, 10.f }, frustumNearFar[2] = {0.001f, 1000.0f};
    static float fovY = glm::pi<float>() * 0.33f;
    while (!glfwWindowShouldClose(window.window))
    {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window.window, &width, &height);

        float aspectRatio;
        aspectRatio = static_cast<float>(width) / static_cast<float>(height);
 
        GL_CHECK(glViewport(0, 0, width, height));

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            static int counter;
            static bool isWireframeRendering = false;
            ImGui::Begin("Hello, world!");
            ImGui::SliderFloat("rotation", &modelRotationRadians, -glm::pi<float>(), glm::pi<float>());
            ImGui::SliderFloat("scale", &modelScale, 0.0001f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat3("translation", glm::value_ptr(modelTranslation), -1000.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("field of view", &fovY, 0.0f, glm::pi<float>());
            //ImGui::SliderFloat2("frustum width/height", frustumWidthHeight, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
            frustumNearFar[0] = std::min(frustumNearFar[0], frustumNearFar[1]);
            ImGui::ColorEdit3("clear color", glm::value_ptr(clear_color));
            ImGui::Checkbox("Wireframe", &isWireframeRendering);
            ImGui::End();
            glPolygonMode(GL_FRONT_AND_BACK, isWireframeRendering ? GL_LINE : GL_FILL);
        }

        GL_CHECK(glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

        glm::mat4x4 с(1.0f);
        с = glm::rotate(с, modelRotationRadians, glm::vec3(1.f, 0.f, 0.f));
        с = glm::scale(с, glm::vec3(modelScale));
        с = glm::translate(с, modelTranslation);
        с = glm::affineInverse(с);
        float frustumBoundY = frustumWidthHeight[1] * 0.5f;
        float frustumBoundX = frustumWidthHeight[0] * 0.5f;
        glm::mat4 p = glm::perspective(fovY, aspectRatio, frustumNearFar[0], frustumNearFar[1]);
        //glm::mat4 p = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, frustumNearFar[0], frustumNearFar[1]);
        glm::mat4 mvp = p * с;
        GL_CHECK(glUseProgram(program.handle));
        GL_CHECK(glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp)));
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        GL_CHECK(glFrontFace(GL_CCW));
        GL_CHECK(glCullFace(GL_BACK));
        GL_CHECK(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(glyphMesh.faceIndices.size()), GL_UNSIGNED_SHORT, nullptr));

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window.window);
    }
 
    return 0;
}