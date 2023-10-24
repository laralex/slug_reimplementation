#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define TTF_FONT_PARSER_IMPLEMENTATION 1
#include <ttfparser.h>
#pragma clang diagnostic pop

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <iostream>
 
struct VertexData
{
    float x, y;
    float u, v;
};

static const char* vertexShaderText =
"#version 330\n"
"uniform mat4 MVP;\n"
"in vec2 vPos;\n"
"in vec2 vUv;\n"
"out vec2 uv;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    uv = vUv;\n"
"}\n";
 
static const char* fragmentShaderText =
"#version 330\n"
"in vec2 uv;\n"
"out vec4 frag;\n"
"void main()\n"
"{\n"
"    frag = vec4(uv.x, uv.y, 0.0, 1.0);\n"
"}\n";
 
static void ErrorCallback(int error, const char* description)
{
    (void)error;
    std::cerr << "Error: " << description << '\n';
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
 
static void OnFontParsed(void* args, void* _font_data, int error) {
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

MeshData ConvertGlyphToMesh(TTFFontParser::Glyph const& glyph) {
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
int main(void)
{
	TTFFontParser::FontData fontData;
    {
        uint8_t condition;
	    int8_t error = TTFFontParser::parse_file("./assets/fonts/HHSamuel.ttf", &fontData, OnFontParsed, &condition);
        if (error) {
            exit(EXIT_FAILURE);
        }

    }

    uint32_t glyphCode = 321;
    std::wcout << L"Parsing glyph: " <<(wchar_t)glyphCode << '\n';
    MeshData glyphMesh = ConvertGlyphToMesh(fontData.glyphs[glyphCode]);

    GLFWwindow* window;
    GLuint vao, vbo, ebo, vertexShader, fragmentShader, program;
    GLint mvpLocation, vposLocation, vuvLocation;
    GLint success;
    char infoLog[512];
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(640, 480, "Simple example", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, KeyCallback);
 
    glfwMakeContextCurrent(window);
        glfwSetErrorCallback(ErrorCallback);
    int versionGl = gladLoadGL(glfwGetProcAddress);
    std::cout << "Loaded OpenGL " << GLAD_VERSION_MAJOR(versionGl) << '.' << GLAD_VERSION_MINOR(versionGl) << '\n';
 
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
 
    // NOTE: OpenGL error checks have been omitted for brevity
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glyphMesh.vertexAttributes[0]) * glyphMesh.vertexAttributes.size(), glyphMesh.vertexAttributes.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glyphMesh.faceIndices[0]) * glyphMesh.faceIndices.size(), glyphMesh.faceIndices.data(), GL_STATIC_DRAW);
 
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderText, nullptr);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
 
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderText, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
 
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
 
    mvpLocation = glGetUniformLocation(program, "MVP");
    vposLocation = glGetAttribLocation(program, "vPos");
    vuvLocation = glGetAttribLocation(program, "vUv");
 
    GLuint vposLocationUnsigned = static_cast<GLuint>(vposLocation);
    GLuint vuvLocationUnsigned = static_cast<GLuint>(vuvLocation);
    glEnableVertexAttribArray(vposLocationUnsigned);
    glVertexAttribPointer(vposLocationUnsigned, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), nullptr);
    glEnableVertexAttribArray(vuvLocationUnsigned);
    glVertexAttribPointer(vuvLocationUnsigned, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), reinterpret_cast<const void*>(sizeof(float) * 2));
 
    glm::vec4 clear_color (0.0f, 0.0f, 0.0f, 1.00f);
    static float modelRotationRadians = 0.0f, modelScale = 1.0f;
    static glm::vec3 modelTranslation = glm::vec3(0.f, 0.f, 500.f);
    static float frustumWidthHeight[2] = { 10.f, 10.f }, frustumNearFar[2] = {0.001f, 1000.0f};
    static float fovY = glm::pi<float>() * 0.33f;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float aspectRatio;
        aspectRatio = static_cast<float>(width) / static_cast<float>(height);
 
        glViewport(0, 0, width, height);

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

        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

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
        glUseProgram(program);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(glyphMesh.faceIndices.size()), GL_UNSIGNED_SHORT, nullptr);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        
    }
 
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
 
    glfwTerminate();
    return 0;
}