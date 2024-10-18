#pragma once

#include <string_view>
#include <string>
#include <toml++/toml.hpp>
#include <cmath>
#include <vector>
#include <map>
#include <tuple>
#include <arpa/inet.h>

#include "imgui/imgui.h"
#include "implot/implot.h"
#include "socket.hpp"
#include "dev.hpp"

namespace plot {
class Var {
public:
    explicit Var(const std::string_view &name, const toml::table &cfg);
    ~Var() = default;

    const std::string name;
    const ImVec4 color;
    const int width;
    const bool filled;
    const int buffer_size;
    ImVector<ImVec2> data;
    int offset = 0;

    void addPoint(float x, float y);
    void erase();
};

class Group {
public:
    explicit Group(const std::string_view &name, const toml::table &cfg);
    ~Group() = default;

    const std::string name;
    const int port;
    const int height_of_each;
    const bool use_gui_time;
    float time = 0.0f;
    float history = 5.0f;
    float history_max = 30.0f;
    std::vector<Var> vars;
    std::vector<std::tuple<std::string, int>> graph_table;
};

class VarGetter: public dev::Dev<io::Socket> {
public:
    explicit VarGetter(io::Socket &socket, const std::tuple<in_addr_t, int> &io_key);
    ~VarGetter() override = default;

    std::map<int, Group *> groups;

private:
    bool unpack(const char *data, const int len) override;
};

class Plot {
public: 
    explicit Plot(io::Socket &socket, const toml::table &cfg);
    ~Plot() = default;

    void plot();

private:
    const bool use_tab;
    bool title = false;
    bool pause = false;
    bool t_sync = true;
    bool auto_fit = true;
    bool link_axis_x = true;
    float history_sync = 5.0f;
    float history_sync_max = 30.0f;
    ImPlotRange lims {0.0, 5.0};
    std::vector<Group> groups;

    const io::Socket &socket;
    std::vector<VarGetter> var_getters;

    void plotGroup(Group &group);
    void plotVar(Var &Var);
};


}
