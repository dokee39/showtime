#pragma once

#include <string_view>
#include <string>
#include <toml++/toml.hpp>
#include <cmath>
#include <vector>
#include <map>

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
    explicit Group(const std::string_view &name,
                   const toml::table &cfg,
                   const bool &t_sync,
                   const float &history_sync,
                   bool &pause,
                   bool &auto_fit,
                   ImPlotRange &lims);
    ~Group() = default;

    const std::string name;
    const int port;
    const int height_of_each;
    float history = 5.0f;
    float history_max = 30.0f;
    std::vector<Var> vars;
    std::map<std::string, int> var_table;

private:
    const bool &t_sync;
    const float &history_sync;
    const bool &pause;
    const bool &auto_fit;
    ImPlotRange &lims;
};

class VarGetter: public dev::Dev<io::Socket> {
public:
    explicit VarGetter(const std::string &name, io::Socket &socket, const std::tuple<std::string_view, int> &io_key):
        Dev(name, socket, io_key) {
        
        
    }

private:
};

class Plot {
public: 
    explicit Plot(const toml::table &cfg);
    /*explicit Plot(const io::Socket &socket, const toml::table &cfg);*/
    ~Plot() = default;

    void plot();

private:
    const bool use_tab;
    bool pause = false;
    bool t_sync = true;
    bool auto_fit = true;
    bool link_axis_x = true;
    float history_sync = 5.0f;
    float history_sync_max = 30.0f;
    ImPlotRange lims {0.0, 5.0};
    std::vector<Group> groups;

    /*const io::Socket &socket;*/
    std::vector<VarGetter> var_getters;

    void plotGroup(Group &group, bool pause, bool auto_fit);
    void plotGroup(Group &group);
    void plotVar(Var &Var);
};


}
