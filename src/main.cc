#include <iostream>

#include "ui.hpp"
#include "plot.hpp"
#include "default_param.hpp"

int main(int, char**)
{
    toml::table cfg;
    plot::Plot* plot;
    try {
        cfg = toml::parse_file("./showtime.toml");
        plot = new plot::Plot(cfg);
    }catch (const toml::parse_error &err) {
        std::cerr << R"(Parsing your config failed!)" << std::endl;
        std::cerr << err << std::endl;
        exit(1);
    }

    GLFWwindow *window = ui::glfwWindowInit();
    ImGuiIO &io = ui::imguiInit(window, cfg["font_size"].value_or(param::FONT_SIZE));

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
        plot->plot();
        ui::loopEnd(window, io);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    ui::cleanup(window);
    delete plot;

    return 0;
}
