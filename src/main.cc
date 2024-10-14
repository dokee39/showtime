#include "ui.hpp"
#include "plot.hpp"

int main(int, char**)
{
    GLFWwindow *window = ui::glfwWindowInit();
    ImGuiIO &io = ui::imguiInit(window);

    auto plot = new plot::Plot();

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

        if (ImGui::BeginTabBar("ShowtimeTabs")) {
            plot->plot();
            ImGui::EndTabBar();
        }
        
        ui::loopEnd(window, io);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    ui::cleanup(window);
    delete plot;

    return 0;
}
