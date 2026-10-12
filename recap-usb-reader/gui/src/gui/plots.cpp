#include "plots.hpp"

#include "misc.hpp"

#include <recap/usb-reader/record.hpp>

#include <IconsFontAwesome6.h>
#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <vector>

using recap::app::device::Device;
using recap::record::Header;
using recap::record::Sample;
using namespace std::string_literals;

#define ICON_HIDE ICON_FA_EYE_SLASH
#define ICON_SHOW ICON_FA_EYE
#define ICON_ENABLE_AUTOFIT ICON_FA_ARROW_DOWN_UP_LOCK
#define ICON_DISABLE_AUTOFIT ICON_FA_ARROW_DOWN_UP_ACROSS_LINE

namespace recap::app::gui::plots {

constexpr unsigned int TimeSpan = 10;

namespace {

class CircularBuffer {
public:
    using buffer_t = std::vector<float>;

    explicit CircularBuffer(buffer_t::size_type n) : m_buf(n, NAN), m_span(n) {}

    void add_point(buffer_t::value_type val)
    {
        m_buf[m_windex] = val;
        m_windex = (m_windex + 1) % m_span;
    }

    const buffer_t::value_type *data() const { return m_buf.data(); }

    buffer_t::size_type size() const { return m_windex; }

    buffer_t::size_type span() const { return m_span; }

private:
    std::size_t m_windex = 0;
    buffer_t m_buf;
    buffer_t::size_type m_span;
};

class Plot {
public:
    bool enabled;

    Plot(
        const std::string& name,
        const std::string& unit,
        double ymin,
        double ymax,
        bool enabled = true,
        bool autofit = true
    ) :
        enabled(enabled),
        m_name(name),
        m_ylabel(build_ylabel(name, unit)),
        m_ymin(ymin),
        m_ymax(ymax),
        m_yaxis_flags(autofit ? ImPlotAxisFlags_AutoFit : 0)
    {}

    void draw(
        float height,
        const CircularBuffer& time,
        const CircularBuffer& yvals,
        const CircularBuffer *events
    )
    {
        ImGui::PushID(this);
        if (enabled) {
            ImGui::BeginGroup();
            enabled = !draw_close_button();
            if (draw_autofit_button()) {
                toggle_autofit_yaxis();
            }
            ImGui::EndGroup();
        }
        if (enabled) {
            draw_plot(height, time, yvals, events);
        }
        ImGui::PopID();
    }

    const std::string& name() const { return m_name; }

private:
    static std::string build_ylabel(const std::string& name, const std::string& unit)
    {
        return name + " (" + unit + ")";
    }

    void draw_plot(
        float height,
        const CircularBuffer& time,
        const CircularBuffer& yvals,
        const CircularBuffer *events
    )
    {
        ImGui::SameLine();
        if (ImPlot::BeginPlot("##plot", ImVec2(-1, height), ImPlotFlags_NoLegend)) {
            ImPlot::SetupAxes("Time (s)", m_ylabel.c_str(), 0, m_yaxis_flags);
            const auto start_time = time.data()[0];
            const auto end_time = start_time + TimeSpan;
            ImPlot::SetupAxisLimits(ImAxis_X1, start_time, end_time, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, m_ymin, m_ymax, ImGuiCond_Once);
            ImPlot::PlotLine(
                "##line", time.data(), yvals.data(), yvals.size(), ImPlotLineFlags_SkipNaN, 0
            );
            if (events) {
                ImPlot::PlotDigital("##event", time.data(), events->data(), events->size(), 0, 0);
            }

            // Some flags may have been set via the plot context menu, so get them here
            m_yaxis_flags = ImPlot::GetCurrentPlot()->Axes[ImAxis_Y1].Flags;

            ImPlot::EndPlot();
        }
    }

    static bool draw_close_button()
    {
        recap::app::gui::PushButtonColor(ImColor(0, 0, 0, 0)); // transparent background
        const bool clicked = ImGui::Button(ICON_HIDE);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && ImGui::BeginTooltip()) {
            ImGui::TextUnformatted("Hide plot");
            ImGui::EndTooltip();
        }
        recap::app::gui::PopButtonColor();
        return clicked;
    }

    bool autofit_yaxis() const { return m_yaxis_flags & ImPlotAxisFlags_AutoFit; }

    void toggle_autofit_yaxis() { m_yaxis_flags ^= ImPlotAxisFlags_AutoFit; }

    bool draw_autofit_button() const
    {
        recap::app::gui::PushButtonColor(ImColor(0, 0, 0, 0)); // transparent background
        const bool autofit = autofit_yaxis();
        const char *const icon = autofit ? ICON_DISABLE_AUTOFIT : ICON_ENABLE_AUTOFIT;
        const bool clicked = ImGui::Button(icon);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && ImGui::BeginTooltip()) {
            ImGui::TextUnformatted(autofit ? "Disable auto-fit" : "Enable auto-fit");
            ImGui::EndTooltip();
        }
        recap::app::gui::PopButtonColor();
        return clicked;
    }

    std::string m_name;
    std::string m_ylabel;
    double m_ymin;
    double m_ymax;
    ImPlotAxisFlags m_yaxis_flags;
};

}; // namespace

struct PlotsView::Impl {
    explicit Impl(Device& device) :
        m_device(device),
        m_new_recording_sub(device.subscribe_new_recording([this](auto, const auto& header) {
            m_recording_view = std::make_unique<RecordingView>(this->m_device, m_plots, header);
        })),
        m_end_recording_sub(device.subscribe_end_recording([this]() { m_recording_view.reset(); })),
        m_disconnected_sub(device.subscribe_disconnected([this](auto) { m_recording_view.reset(); })
        )
    {}

    enum class Channel {
        Pressure,
        Flow,
        Mask,
        Ear,
        H2S,
        H2STemp,
        H2SHumidity,
    };

    void draw()
    {
        if (m_recording_view) {
            m_recording_view->draw();
        }
    }

private:
    using PlotsMap = std::map<Channel, Plot>;

    struct RecordingView {
        RecordingView(Device& device, PlotsView::Impl::PlotsMap& plots, const Header& header) :
            m_device(device),
            m_plots(plots),
            m_new_sample_sub(device.subscribe_new_sample([this](const auto *sample) {
                add_sample(sample);
            })),
            m_header(header),
            m_header_str(header.buffer_str()),
            m_version_str(header.firmware_version().str()),
            m_event_buf(header.sample_rate_khz() * TimeSpan * 1000),
            m_time_buf(header.sample_rate_khz() * TimeSpan * 1000)
        {
            for (const auto& plot : plots) {
                m_plot_buffers.insert(
                    {std::get<0>(plot), CircularBuffer(header.sample_rate_khz() * TimeSpan * 1000)}
                );
            }
        }

        void draw()
        {
            ImGui::Text(
                "Version: %s, audio sample rate: %u kHz",
                m_version_str.c_str(),
                m_header.sample_rate_khz()
            );
            ImGui::SameLine();
            if (ImGui::Button(
                    show_header ? ICON_HIDE " Hide header fields" :
                                  ICON_SHOW " Show all header fields"
                ))
            {
                show_header = !show_header;
            }
            if (show_header) {
                ImGui::TextUnformatted(m_header_str.data());
            }

            if (ImGui::BeginCombo("Plots", enabled_plots_str().c_str())) {
                for (auto& pair : m_plots) {
                    auto& plot = std::get<1>(pair);
                    ImGui::Checkbox(plot.name().c_str(), &plot.enabled);
                }
                ImGui::EndCombo();
            }

            auto num_enabled = get_num_enabled();
            ImGui::SameLine();
            {
                ImGui::BeginDisabled(num_enabled == m_plots.size());
                if (ImGui::Button(ICON_SHOW "  Show all")) {
                    show_all_plots();
                    num_enabled = m_plots.size();
                }
                ImGui::EndDisabled();
            }
            ImGui::SameLine();
            {
                ImGui::BeginDisabled(num_enabled == 0);
                if (ImGui::Button(ICON_HIDE "  Hide all")) {
                    hide_all_plots();
                    num_enabled = 0;
                }
                ImGui::EndDisabled();
            }
            ImGui::SameLine();
            ImGui::Checkbox("Show events", &show_events);
            ImGui::SameLine();
            ImGui::Checkbox("Allow event override", &allow_event_override);

            const float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
            const float event_button_height =
                allow_event_override ? (ImGui::GetTextLineHeight() + item_spacing_y) * 2 : 0;

            const float height =
                (ImGui::GetContentRegionAvail().y - event_button_height - item_spacing_y)
                    / num_enabled
                - item_spacing_y;
            for (auto& [channel, plot] : m_plots) {
                plot.draw(
                    height,
                    m_time_buf,
                    m_plot_buffers.at(channel),
                    show_events ? &m_event_buf : nullptr
                );
            }

            if (allow_event_override) {
                const bool space_down = ImGui::IsKeyDown(ImGuiKey_Space);
                // Adds red button, if space key pushed show as active regardless of actual button
                // state
                constexpr float EventOverrideButtonHue = 0; // red
                if (space_down) {
                    PushButtonColor(ButtonColorActive(EventOverrideButtonHue));
                } else {
                    PushButtonColorHSV(EventOverrideButtonHue);
                }
                ImGui::Button(
                    "Event [space]", {ImGui::GetContentRegionAvail().x, event_button_height}
                );
                m_device.set_event_override(space_down || ImGui::IsItemActive());
                PopButtonColor();
            }
        }

    private:
        Device& m_device;
        PlotsMap& m_plots;
        Device::NewSampleObservable::Observer m_new_sample_sub;
        Header m_header;
        std::string m_header_str;
        std::string m_version_str;
        CircularBuffer m_event_buf;
        CircularBuffer m_time_buf;
        std::map<PlotsMap::key_type, CircularBuffer> m_plot_buffers;

        bool show_events = true;
        bool allow_event_override = true;
        bool show_header = false;

        void add_sample(const Sample *sample)
        {
            using enum Channel;
            m_plot_buffers.at(Ear).add_point((float) sample->ear_volts());
            m_plot_buffers.at(Mask).add_point((float) sample->mask_volts());
            add_optional_sample(m_plot_buffers.at(Flow), sample->flow_lpm());
            add_optional_sample(m_plot_buffers.at(Pressure), sample->pressure_pascals());
            add_optional_sample(m_plot_buffers.at(H2S), sample->h2s_conc_ppb());
            add_optional_sample(m_plot_buffers.at(H2STemp), sample->h2s_temp_celsius());
            add_optional_sample(m_plot_buffers.at(H2SHumidity), sample->h2s_humidity_percent());
            m_event_buf.add_point(sample->event() ? 1 : NAN);
            m_time_buf.add_point((float) sample->millis() / 1000);
        }

        void show_all_plots()
        {
            for (auto& plot : m_plots) {
                std::get<1>(plot).enabled = true;
            }
        }

        void hide_all_plots()
        {
            for (auto& plot : m_plots) {
                std::get<1>(plot).enabled = false;
            }
        }

        template <class T>
        static inline void
        add_optional_sample(CircularBuffer& buffer, const std::optional<T>& sample)
        {
            buffer.add_point(sample ? static_cast<float>(*sample) : NAN);
        }

        PlotsMap::size_type get_num_enabled() const
        {
            return std::count_if(m_plots.begin(), m_plots.end(), [](const auto& p) {
                return std::get<1>(p).enabled;
            });
        }

        std::string enabled_plots_str() const
        {
            std::string s;
            for (const auto& pair : m_plots) {
                const auto& plot = std::get<1>(pair);
                if (plot.enabled) {
                    if (!s.empty()) {
                        s.append(", ");
                    }
                    s.append(plot.name());
                }
            }
            if (s.empty()) {
                return "(None selected)";
            }
            return s;
        }
    };

    PlotsMap m_plots = {
        {Channel::Pressure, {"Pressure", "Pa", -500, 500}},
        {Channel::Flow, {"Flow", "L/min", -220, 220}},
        {Channel::Mask, {"Mask audio", "V", -0.2, 5.2, true, false}},
        {Channel::Ear, {"Ear audio", "V", -0.2, 5.2, true, false}},
        {Channel::H2S, {"H2S", "ppb", -500, 500, false}},
        {Channel::H2STemp, {"H2S temp", "°C", 0, 60, false}},
        {Channel::H2SHumidity, {"H2S humidity", "%", 0, 100, false}},
    };

    Device& m_device;
    Device::NewRecordingObservable::Observer m_new_recording_sub;
    Device::EndRecordingObservable::Observer m_end_recording_sub;
    Device::DisconnectedObservable::Observer m_disconnected_sub;
    std::unique_ptr<RecordingView> m_recording_view;
};

PlotsView::PlotsView(Device& device) : m_pimpl(std::make_unique<Impl>(device)) {}

PlotsView::~PlotsView() = default;

void PlotsView::draw()
{
    m_pimpl->draw();
}

}; // namespace recap::app::gui::plots
