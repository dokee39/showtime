#include <iostream>
#include <sys/socket.h>

#include "ui.hpp"
#include "plot.hpp"
#include "default_param.hpp"
#include "socket.hpp"

int main(int, char**)
{
    toml::table cfg;
    plot::Plot* plot;
    io::Socket* socket;
    try {
        cfg = toml::parse_file("./showtime.toml");
        socket = new io::Socket("socket", cfg["port"].value_or(param::SOCKET_PORT), cfg["socket_buffer_size"].value_or(param::SOCKET_BUFFERSIZE));
        plot = new plot::Plot(*socket, cfg);
    }catch (const toml::parse_error &err) {
        std::cerr << R"(Parsing your config failed!)" << std::endl;
        std::cerr << err << std::endl;
        exit(1);
    }

    GLFWwindow *window = ui::glfwWindowInit();
    ImGuiIO &io = ui::imguiInit(window, cfg["font_size"].value_or(param::FONT_SIZE));

    socket->run();

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

    socket->stop();
    delete socket;
    delete plot;

    return 0;
}
