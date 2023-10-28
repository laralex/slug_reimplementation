#include "window.hpp"

#include <iostream>

GlfwWindowHandle::~GlfwWindowHandle() {
   if (window != nullptr) {
      std::cout << "Glfw deleter" << std::endl;
      glfwDestroyWindow(window);
      glfwTerminate();
   }
}

auto initGlfw() -> std::variant<GlfwWindowHandle, const char*> {
    GLFWwindow* window;
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);

    window = glfwCreateWindow(640, 480, "Simple example", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return "Failed glfwCreateWindow";
    }
 
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    return GlfwWindowHandle{window};
}

auto initImgui(GLFWwindow* window) -> ImguiContext {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    return ImguiContext{(void*)1};
}
