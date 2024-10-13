#pragma once
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/misc/freetype/imgui_freetype.h"
#include "implot/implot.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>


namespace ui {
GLFWwindow *glfwWindowInit();
ImGuiIO &imguiInit(GLFWwindow* window);
void loopStart(GLFWwindow *window, ImGuiIO &io);
void loopEnd(GLFWwindow *window, ImGuiIO &io);
void cleanup(GLFWwindow *window);
}

