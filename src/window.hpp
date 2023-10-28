#pragma once

// TODO: move into cpp, provide full abstraction
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// TODO: move into cpp, provide full abstraction
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <variant>
#include <memory>

struct GLFWwindow;

struct GlfwWindowHandle {
   GLFWwindow* window;
   ~GlfwWindowHandle();
   GlfwWindowHandle(GLFWwindow* window = 0) : window(window) {};
   GlfwWindowHandle(GlfwWindowHandle&& rhs) : window(rhs.window) { rhs.window = nullptr; };
   GlfwWindowHandle& operator=(GlfwWindowHandle&& rhs) {
      window = rhs.window; rhs.window = 0; return *this; }
};

struct ImguiDeleter {
   void operator()(void*) const {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
   }
};
using ImguiContext = std::unique_ptr<void, ImguiDeleter>;

auto initGlfw() -> std::variant<GlfwWindowHandle, const char*>;

auto initImgui(GLFWwindow* window) -> ImguiContext;
