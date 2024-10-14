#include <iostream>

#include "plot.hpp"
#include "implot/implot.h"

namespace plot {
Var::Var(const std::string_view &name, const toml::table &cfg):
    name(name),
    color(cfg["color"][0].value_or(0), cfg["color"][1].value_or(0), cfg["color"][2].value_or(0), cfg["color"][3].value_or(-1)),
    width(cfg["width"].value_or(0.5f)),
    filled(cfg["filled"].value_or(false)),
    buffer_size(cfg["buffer_size"].value_or(2000)) {
    data.reserve(buffer_size);
}

void Var::addPoint(float x, float y) {
    if (data.size() < buffer_size)
        data.push_back(ImVec2(x,y));
    else {
        data[offset] = ImVec2(x,y);
        offset =  (offset + 1) % buffer_size;
    }
}

void Var::erase() {
    if (data.size() > 0) {
        data.shrink(0);
        offset  = 0;
    }
}

Tab::Tab(const std::string_view &name, const toml::table &cfg):
    name(name),
    port(cfg["port"].value_or(14514)) {
    for (const auto &[key, value]: cfg) {
        if (!value.is_table()) {
            continue;
        }
        var_table.insert(std::pair<std::string, int>(key, value.as_table()->size()));
        for (const auto &[key_, value_]: *value.as_table()) {
            if (!value_.is_table()) {
                std::cerr << "Error config: " << key << "." << key_ << std::endl;
                exit(1);
            }
            vars.emplace_back(key_, *value_.as_table());
        }
    }
}

Plot::Plot(const std::string &cfg_path) {
    try {
        auto cfg = toml::parse_file(cfg_path);
        for (const auto &[key, value]: cfg) {
            if (!value.is_table()) {
                std::cerr << "Error config: " << key << std::endl;
                exit(1);
            }
            tabs.emplace_back(key, *value.as_table());
        }
    }catch (const toml::parse_error &err) {
        std::cerr << R"(Parsing your config ")" + cfg_path + R"(" failed!)" << std::endl;
        std::cerr << err << std::endl;
        exit(1);
    }
}

void Plot::plot() {
    // use_tab_or_tree   history  auto_fit   height
    const ImPlotAxisFlags flags = ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    for (auto &tab: tabs) {
        if (!ImGui::BeginTabItem(tab.name.c_str())) {
            continue;
        }
        int var_id = 0;
        for (const auto &[key, value]: tab.var_table) {
            if (!ImPlot::BeginPlot(key.c_str(), ImVec2(-1,300))) {
                continue;
            }
            ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
            ImVec2 mouse = ImGui::GetMousePos();
            static float t = 0;
            t += ImGui::GetIO().DeltaTime;
            static float history = 10.0f;
            ImGui::SliderFloat("History",&history,1,30,"%.1f s");
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);

            for (int i = 0; i < value; i++) {
                tab.vars[var_id].addPoint(t, mouse.y * 0.0005f);
                ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, tab.vars[var_id].width);
                if (tab.vars[var_id].filled) {
                    ImPlot::PlotShaded(tab.vars[var_id].name.c_str(), 
                                       &tab.vars[var_id].data[0].x, 
                                       &tab.vars[var_id].data[0].y, 
                                       tab.vars[var_id].data.size(), 
                                       -INFINITY, 
                                       0, 
                                       tab.vars[var_id].offset, 
                                       2 * sizeof(float));
                } else {
                    ImPlot::PlotLine(tab.vars[var_id].name.c_str(), 
                                     &tab.vars[var_id].data[0].x, 
                                     &tab.vars[var_id].data[0].y, 
                                     tab.vars[var_id].data.size(), 
                                     0, 
                                     tab.vars[var_id].offset, 
                                     2 * sizeof(float));
                }
                var_id++;
            }
            ImPlot::EndPlot();
        }
        ImGui::EndTabItem();
    }
}
}

