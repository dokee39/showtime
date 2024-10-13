#include <cmath>
#include "ui.hpp"

struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 5000) {
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

// Main code
int main(int, char**)
{
    GLFWwindow *window = ui::glfwWindowInit();
    ImGuiIO &io = ui::imguiInit(window);

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
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ui::loopStart(window, io);

        if (ImGui::BeginTabBar("ImPlotDemoTabs")) {
            if (ImGui::BeginTabItem("Plots")) {
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
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        
        ui::loopEnd(window, io);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    ui::cleanup(window);

    return 0;
}
