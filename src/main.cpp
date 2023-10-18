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
    float r, g, b;
};

static const VertexData vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};
 
static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"in vec3 vCol;\n"
"in vec2 vPos;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 frag;\n"
"void main()\n"
"{\n"
"    frag = vec4(color, 1.0);\n"
"}\n";
 
static void error_callback(int error, const char* description)
{
    (void)error;
    std::cerr << "Error: " << description << '\n';
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
 
static void on_font_parsed(void* args, void* _font_data, int error) {
    if (error) {
		*(uint8_t*)args = error;
		std::cout << "Unable to parse font";
        return;
	}
	TTFFontParser::FontData* font_data = (TTFFontParser::FontData*)_font_data;
    printf("Font: %s %s parsed\n", font_data->font_names.begin()->font_family.c_str(), font_data->font_names.begin()->font_style.c_str());
    printf("Number of glyphs: %d\n", uint32_t(font_data->glyphs.size()));

    //step through glyph geometry
    {
        size_t num_glyphs_to_print = 20;
        for (const auto& glyph_iterator : font_data->glyphs) {
            if (num_glyphs_to_print-- <= 0) {
                break;
            }
            uint32_t num_curves = 0, num_lines = 0;
            for (const auto& path_list : glyph_iterator.second.path_list) {
                for (const auto& geometry : path_list.geometry) {
                    if (geometry.is_curve)
                        num_curves++;
                    else
                        num_lines++;
                }
            }
            std::wcout << "glyph " << static_cast<wchar_t>(glyph_iterator.first) << " : n_curves=" << num_curves << " n_lines=" << num_lines << '\n';
        }
    }

    *(uint8_t*)args = 1;
}

int main(void)
{
	TTFFontParser::FontData font_data;
    {
        uint8_t condition;
	    int8_t error = TTFFontParser::parse_file("./assets/fonts/HHSamuel.ttf", &font_data, on_font_parsed, &condition);
        if (error) {
            exit(EXIT_FAILURE);
        }
    }

    GLFWwindow* window;
    GLuint vao, vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
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
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
        glfwSetErrorCallback(error_callback);
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
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, nullptr);
    glCompileShader(vertex_shader);

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
 
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }
 
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
 
    GLuint vpos_location_unsigned = static_cast<GLuint>(vpos_location);
    GLuint vcol_location_unsigned = static_cast<GLuint>(vcol_location);
    glEnableVertexAttribArray(vpos_location_unsigned);
    glVertexAttribPointer(vpos_location_unsigned, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), nullptr);
    glEnableVertexAttribArray(vcol_location_unsigned);
    glVertexAttribPointer(vcol_location_unsigned, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), reinterpret_cast<const void*>(sizeof(float) * 2));
 
    glm::vec4 clear_color (0.45f, 0.55f, 0.60f, 1.00f);
    float rotation_radians;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float ratio;
        int width, height;
 
        glfwGetFramebufferSize(window, &width, &height);
        ratio = static_cast<float>(width) / static_cast<float>(height);
 
        glViewport(0, 0, width, height);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            static int counter;
            ImGui::Begin("Hello, world!");
            ImGui::Text("This is some useful text.");
            ImGui::SliderFloat("rotation", &rotation_radians, 0.0f, 2.f * glm::pi<float>());
            ImGui::ColorEdit3("clear color", glm::value_ptr(clear_color));
            if (ImGui::Button("Button"))
                counter++;
            ImGui::End();
        }

        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4x4 m(1.0f);
        m = glm::rotate(m, rotation_radians, glm::vec3(0.f, 0.f, 1.f));
        glm::mat4 p = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glm::mat4 mvp = p * m;
 
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glDrawArrays(GL_TRIANGLES, 0, 3);

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