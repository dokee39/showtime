#include <stdio.h>
#include <cmath>

#include "ui.hpp"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

namespace ui {
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    
    // 更新 ImGui 的缩放系数
    ImGuiIO& io = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2(xscale, yscale);
    /*io.FontGlobalScale = xscale;*/
}

GLFWwindow *glfwWindowInit() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        exit(1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "showtime", nullptr, nullptr);
    if (window == nullptr)
        exit(1);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glEnable(GL_MULTISAMPLE);

    return window;
}

ImGuiIO &imguiInit(GLFWwindow* window) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 100;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_ForceAutoHint;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bitmap;
    ImFont* font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/TTF/HackNerdFontMono-Regular.ttf", 18.0f, &cfg);
    IM_ASSERT(font != nullptr);

    return io;
}

void loopStart(GLFWwindow *window, ImGuiIO &io) {
    static int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));

    ImGui::Begin("showtime", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::Text("SHOWTIME says hello. (@dokee, powered by ImPlot: %s)", IMPLOT_VERSION);
    
    static bool showWarning = sizeof(ImDrawIdx)*8 == 16 && (ImGui::GetIO().BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset) == false;
    if (showWarning) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
        ImGui::TextWrapped("WARNING: ImDrawIdx is 16-bit and ImGuiBackendFlags_RendererHasVtxOffset is false. Expect visual glitches and artifacts! See README for more information.");
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
}

void loopEnd(GLFWwindow *window, ImGuiIO &io) {
    const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 text_size = ImGui::CalcTextSize("FPS: xxx.x");
    
    ImGui::SetCursorPos(ImVec2(window_size.x - text_size.x - 10, window_size.y - text_size.y - 10));
    ImGui::Text("FPS %.1f", io.Framerate);

    ImGui::End();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

void cleanup(GLFWwindow *window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
}

