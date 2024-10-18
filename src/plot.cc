#include <netinet/in.h>
#include <sys/socket.h>
#include <tuple>

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

Group::Group(const std::string_view &name, const toml::table &cfg):
    name(name),
    port(cfg["port"].value_or(param::PORT)),
    height_of_each(cfg["height_of_each"].value_or(param::HEIGHT_OF_EACH)),
    use_gui_time(cfg["use_gui_time"].value_or(param::USE_GUI_TIME)) {
    for (const auto &[key, value]: cfg) {
        if (!value.is_table()) {
            continue;
        }
        int var_num = 0;
        auto graph_cfg = *value.as_table();
        for (const auto &[key_, value_]: graph_cfg) {
            if (!value_.is_table()) {
                continue;
            }
            auto var_cfg = *value_.as_table();
            vars.emplace_back(var_cfg["name"].value_or(std::string(key_)), var_cfg);
            var_num++;
        }
        if (var_num) {
            graph_table.emplace_back(std::make_tuple(graph_cfg["name"].value_or(std::string(key)), var_num));
        }
    }
}

VarGetter::VarGetter(io::Socket &socket, const std::tuple<in_addr_t, int> &io_key):
    Dev(std::string(inet_ntoa((in_addr)std::get<0>(io_key))) + ":" + std::to_string(std::get<1>(io_key)), socket, io_key) {
}

bool VarGetter::unpack(const char *data, const int len) {
    struct Data {
        int32_t id;
        float t;
        float* var;
    };

    if (len <= (int)(sizeof(int32_t) + sizeof(float))) {
        return false;
    }

    Data d;
    d.id = *(int32_t *)(data);
    d.t = *(float *)(data + sizeof(int32_t));
    d.var = (float *)(const_cast<char *>(data + sizeof(int32_t) + sizeof(float)));
    int var_size = (len - sizeof(int32_t) - sizeof(float)) / sizeof(float);

    auto it = groups.find(d.id);
    if (it == groups.end()) {
        return false;
    }
    Group &group = *it->second;
    if (var_size != (int)group.vars.size()) {
        return false;
    }

    if (group.use_gui_time) {
        d.t = util::getTime();
    }

    int i = 0;
    for (auto &var: group.vars) {
        var.addPoint(d.t, d.var[i]);
        i++;
    }

    return true;
}

Plot::Plot(io::Socket &socket, const toml::table &cfg): 
    use_tab(cfg["use_tab"].value_or(param::USE_TAB)),
    socket(socket) {
    for (const auto &[key, value]: cfg) {
        if (!value.is_table()) {
            continue;
        }
        auto group_cfg = *value.as_table();
        groups.emplace_back(key, group_cfg);

        in_addr_t ip = util::to_in_addr(cfg["host"].value_or(param::HOST));
        int port = group_cfg["port"].value_or(param::PORT);
        auto vargetter_key = std::make_tuple(ip, port);

        int group_id = group_cfg["id"].value_or(param::GROUP_ID);

        bool is_vargetter_added = false;
        for (auto &var_getter: var_getters) {
            if (vargetter_key == var_getter.getIoKey()) {
                auto it = var_getter.groups.find(group_id);
                if (it != var_getter.groups.end()) {
                    std::cerr << "Groups from the same client has the same id: " << group_id << std::endl;
                    exit(1);
                }
                var_getter.groups.emplace(group_id, &groups.back());
                is_vargetter_added = true;
            }
        }
        if (!is_vargetter_added) {
            var_getters.emplace_back(socket, vargetter_key);
            var_getters.back().groups.emplace(group_id, &groups.back());
        }
    }
}

void Plot::plot() {
    ImGui::SeparatorText("Options");

    bool pause_tmp = pause;
    ImGui::Checkbox("Title", &title);
    ImGui::SameLine();
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

    if (!ImPlot::BeginSubplots("", group.graph_table.size(), 1, ImVec2(-1, group.height_of_each * group.graph_table.size()), pause ? 0 : ImPlotSubplotFlags_LinkAllX)) {
        return;
    }

    int var_id = 0;
    for (const auto &[key, value]: group.graph_table) {
        if (!ImPlot::BeginPlot((group.name + "." + key).c_str(), ImVec2(-1,300), (title ? 0 : ImPlotFlags_NoTitle) | ImPlotFlags_Crosshairs)) {
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

