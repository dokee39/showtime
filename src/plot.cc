#include <iostream>

#include "plot.hpp"
#include "default_param.hpp"
#include "implot/implot.h"
#include "util.hpp"

namespace plot {
Var::Var(const std::string_view &name, const toml::table &cfg):
    name(name),
    color(ImVec4(cfg["color"][0].value_or(0.0f), cfg["color"][1].value_or(0.0f), cfg["color"][2].value_or(0.0f), cfg["color"][3].value_or(-1.0f))),
    width(cfg["width"].value_or(param::WIDTH)),
    filled(cfg["filled"].value_or(param::FILLED)),
    buffer_size(cfg["buffer_size"].value_or(param::BUFFER_SIZE)) {
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

Group::Group(const std::string_view &name,
             const toml::table &cfg,
             const bool &t_sync,
             const float &history_sync,
             bool &pause,
             bool &auto_fit,
             ImPlotRange &lims):
    name(name),
    port(cfg["port"].value_or(param::PORT)),
    height_of_each(cfg["height_of_each"].value_or(param::HEIGHT_OF_EACH)),
    t_sync(t_sync), 
    history_sync(history_sync),
    pause(pause),
    auto_fit(auto_fit),
    lims(lims) {
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

Plot::Plot(const toml::table &cfg): 
    use_tab(cfg["use_tab"].value_or(param::USE_TAB)) {
    for (const auto &[key, value]: cfg) {
        if (!value.is_table()) {
            continue;
        }
        groups.emplace_back(key, *value.as_table(), t_sync, history_sync, pause, auto_fit, lims);
    }
}

void Plot::plot() {
    ImGui::SeparatorText("Options");

    bool pause_tmp = pause;
    ImGui::Checkbox("Pause", &pause);
    ImGui::SameLine();
    ImGui::Checkbox("Time Sync", &t_sync);
    ImGui::SameLine();
    ImGui::Checkbox("Auto Fit Y", &auto_fit);
    if (pause) {
        ImGui::SameLine();
        ImGui::Checkbox("Link Axis X", &link_axis_x);

        if (!pause_tmp) {
            double t = util::getTime();
            lims.Min = t - history_sync;
            lims.Max = t;
        }
    }

    if (t_sync) {
        ImGui::SliderFloat("History", &history_sync, 0.001, history_sync_max, "%.3f s");
        ImGui::SameLine();
        ImGui::InputFloat("max", &history_sync_max, 1.0f, 1.0f, "%.3f s");
    }

    if (use_tab) {
        if (ImGui::BeginTabBar("ShowTabs")) {
            for (auto &group: groups) {
                if (!ImGui::BeginTabItem(group.name.c_str())) {
                    continue;
                }
                plotGroup(group);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    } else {
        ImGui::SeparatorText("Plots");
        for (auto &group: groups) {
            if (!ImGui::TreeNodeEx(group.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                continue;
            }
            plotGroup(group);
            ImGui::TreePop();
        }
    }
}

void Plot::plotGroup(Group &group) {
    const ImPlotAxisFlags flags_x = ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoSideSwitch | ImPlotAxisFlags_RangeFit;
    ImPlotAxisFlags flags_y = flags_x;
    if (auto_fit) {
        flags_y |= ImPlotAxisFlags_AutoFit;
    }

    if (!t_sync) {
        ImGui::SliderFloat("History", &group.history, 0.001, group.history_max, "%.3f s");
        ImGui::SameLine();
        ImGui::InputFloat("max", &group.history_max, 1.0f, 1.0f, "%.3f s");
    } else {
        group.history = history_sync;
    }

    if (!ImPlot::BeginSubplots("", group.var_table.size(), 1, ImVec2(-1, group.height_of_each * group.var_table.size()), ImPlotSubplotFlags_LinkAllX)) {
        return;
    }

    int var_id = 0;
    for (const auto &[key, value]: group.var_table) {
        if (!ImPlot::BeginPlot((group.name + "." + key).c_str(), ImVec2(-1,300),ImPlotFlags_NoTitle | ImPlotFlags_Crosshairs)) {
            continue;
        }
        ImPlot::SetupAxes(nullptr, nullptr, flags_x, flags_y);
        ImVec2 mouse = ImGui::GetMousePos();
        double t = util::getTime();

        if (!pause) {
            ImPlot::SetupAxisLimits(ImAxis_X1, t - group.history, t, ImGuiCond_Always);
        } else {
            ImPlot::SetupAxisLinks(ImAxis_X1, link_axis_x ? &lims.Min : nullptr, link_axis_x ? &lims.Max : nullptr);
        }

        for (int i = 0; i < value; i++) {
            group.vars[var_id].addPoint(t, (i + 1) * mouse.y * 0.0005f);
            plotVar(group.vars[var_id]);
            var_id++;
        }
        ImPlot::EndPlot();
    }
    ImPlot::EndSubplots();
}

void Plot::plotVar(Var &var) {
    ImPlot::SetNextLineStyle(var.color, var.width);
    if (var.filled) {
        ImPlot::SetNextFillStyle(var.color);
        ImPlot::PlotShaded(var.name.c_str(),
                           &var.data[0].x,
                           &var.data[0].y,
                           var.data.size(),
                           0,
                           0,
                           var.offset,
                           2 * sizeof(float));
    } else {
        ImPlot::PlotLine(var.name.c_str(),
                         &var.data[0].x,
                         &var.data[0].y,
                         var.data.size(),
                         0,
                         var.offset,
                         2 * sizeof(float));
    }
}
}

