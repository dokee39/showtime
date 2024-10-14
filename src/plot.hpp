#pragma once

#include <string_view>
#include <string>
#include <toml++/toml.hpp>
#include <cmath>
#include <vector>
#include <map>

#include "imgui.h"
#include "implot/implot.h"

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

class Tab {
public:
    explicit Tab(const std::string_view &name, const toml::table &cfg);
    ~Tab() = default;

    const std::string name;
    const int port;
    std::vector<Var> vars;
    std::map<std::string, int> var_table;
};

class Plot {
public: 
    explicit Plot(const std::string &cfg_path = "./showtime.toml");
    ~Plot() = default;

    std::vector<Tab> tabs;

    void plot();
};
}
