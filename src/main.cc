#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/misc/freetype/imgui_freetype.h"
#include "implot/implot.h"
#include <stdio.h>
#include <cmath>
#include <cstdlib>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

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

struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

void DemoHeader(const char* label, void(*demo)()) {
    if (ImGui::TreeNodeEx(label)) {
        demo();
        ImGui::TreePop();
    }
}

void Demo_RealtimePlots() {
    ImGui::BulletText("Move your mouse to change the data!");
    ImGui::BulletText("This example assumes 60 FPS. Higher FPS requires larger buffer size.");
    static ScrollingBuffer sdata1, sdata2;
    ImVec2 mouse = ImGui::GetMousePos();
    static float t = 0;
    t += ImGui::GetIO().DeltaTime;
    sdata1.AddPoint(t, mouse.x * 0.0005f);
    sdata2.AddPoint(t, mouse.y * 0.0005f);

    static float history = 10.0f;
    ImGui::SliderFloat("History",&history,1,30,"%.1f s");

    static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

    if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1,150))) {
        ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL,0.5f);
        ImPlot::PlotShaded("Mouse X", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), -INFINITY, 0, sdata1.Offset, 2 * sizeof(float));
        ImPlot::PlotLine("Mouse Y", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), 0, sdata2.Offset, 2*sizeof(float));
        ImPlot::EndPlot();
    }
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glEnable(GL_MULTISAMPLE);

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
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 100;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_ForceAutoHint;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bitmap;
    ImFont* font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/TTF/HackNerdFontMono-Regular.ttf", 18.0f, &cfg);
    IM_ASSERT(font != nullptr);

    // Our state
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        static int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));

            ImGui::Begin("ImPlot Demo", &show_another_window, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::Text("ImPlot says hello. (%s)", IMPLOT_VERSION);
            
            static bool showWarning = sizeof(ImDrawIdx)*8 == 16 && (ImGui::GetIO().BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset) == false;
            if (showWarning) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
                ImGui::TextWrapped("WARNING: ImDrawIdx is 16-bit and ImGuiBackendFlags_RendererHasVtxOffset is false. Expect visual glitches and artifacts! See README for more information.");
                ImGui::PopStyleColor();
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Spacing();

            if (ImGui::BeginTabBar("ImPlotDemoTabs")) {
                if (ImGui::BeginTabItem("Plots")) {
                    DemoHeader("Realtime Plots", Demo_RealtimePlots);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

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
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
