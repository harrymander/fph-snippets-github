// Avoids warnings about localtime on MSVC
#define _CRT_SECURE_NO_WARNINGS

#include "controller.hpp"

#include "../filename.hpp"
#include "device.hpp"
#include "directory.hpp"
#include "misc.hpp"
#include "recap/usb-reader/record.hpp"
#include "strhelpers.hpp"
#include "writer-observer.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <magic_enum/magic_enum.hpp>

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

using recap::app::device::Device;
using recap::app::gui::DirectorySelector;
using recap::app::writer_observer::WriterObserver;
using recap::record::CompleteSample;

namespace recap::app::gui::controller {

class Controller::State {
public:
    State(Controller& controller, Device& device) :
        m_controller(controller), m_device(device), m_dir_selector(controller.m_dir_selector)
    {}

    virtual ~State() = default;
    State(State&&) = delete;
    State(const State&) = delete;
    State& operator=(const State&) = delete;
    State& operator=(State&&) = delete;

    virtual void update_and_draw() = 0;

protected:
    Controller& m_controller;
    Device& m_device;
    DirectorySelector& m_dir_selector;

    template <class S, class... Args> void set_state(Args&&...args)
    {
        m_controller.set_state<S>(std::forward<Args>(args)...);
    }

    void set_next_output_path(const std::filesystem::path& path)
    {
        m_controller.m_writer.set_next_output_path(path);
    }

    void clear_next_output_path() { m_controller.m_writer.clear_next_output_path(); }

    WriterObserver::ErrorObservable::Observer
    subscribe_writer_error(const WriterObserver::ErrorObservable::Function& func)
    {
        return m_controller.m_writer.subscribe_error(func);
    }
};

class ConnectedState;
class NotConnectedState;
class WriterErrorState;

class RecordingFilenameForm {
public:
    [[nodiscard]] const char *error() const
    {
        if (free_input()) {
            if (m_extra.empty() || m_extra == FilenameSuffix) {
                return "Missing filename";
            }

            if (!filename::is_valid_filename(m_extra)) {
                return "Invalid filename";
            }
        } else {
            if (m_participant_id == 0) {
                return "Participant ID must be greater than 0";
            }

            if (m_participant_id > MaxParticipantId) {
                static const std::string err =
                    "Participant ID cannot be greater than " + std::to_string(MaxParticipantId);
                return err.c_str();
            }

            if (m_participant_id > 100 && m_participant_id % 100 != 0) {
                return "Healthy participant ID must be divisible by 100";
            }

            if (m_repeat_num == 0) {
                return "Repeat number must be greater than 0";
            }

            if (!(m_extra.empty() || filename::is_valid_filename(m_extra))) {
                return "Invalid extra filename data";
            }
        }

        return nullptr;
    }

    [[nodiscard]] std::string filename() const
    {
        using Clock = std::chrono::system_clock;

        std::stringstream ss;

        if (!free_input() || m_add_datetime) {
            const auto time = Clock::to_time_t(Clock::now());

            // Cannot use colons in time since not supported on Windows
            ss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H%M%S%z-");
        }

        if (free_input()) {
            ss << m_extra;
            if (!m_extra.ends_with(FilenameSuffix)) {
                ss << FilenameSuffix;
            }
        } else {
            ss << 'P' << std::setfill('0') << std::setw(3) << m_participant_id << '-'
               << TestTypeStrings.at(magic_enum::enum_integer(m_test_type)).short_name << '-'
               << m_repeat_num;

            if (m_extra.empty() || m_extra == FilenameSuffix) {
                ss << FilenameSuffix;
            } else {
                ss << '-' << m_extra;
                if (!m_extra.ends_with(FilenameSuffix)) {
                    ss << FilenameSuffix;
                }
            }
        }

        return ss.str();
    }

    void draw()
    {
        static const std::string FilenameExplanation = []() {
            std::stringstream str;
            str << "Filenames cannot be more than " << filename::MAX_FILENAME_LEN
                << " characters long and can only contain letters, numerals, and the symbols: "
                << filename::ALLOWED_FILENAME_SYMBOLS << ".";
            return str.str();
        }();

        if (!free_input()) {
            input_positive_int("ID", m_participant_id);
        }

        if (test_type_combo("Test", m_test_type)) {
            m_repeat_num = 1;
            m_extra.clear();
            m_add_datetime = true;
        }

        if (!free_input()) {
            input_positive_int("Repeat", m_repeat_num);
        }

        ImGui::InputText(free_input() ? "Filename" : "Extra", &m_extra);
        ImGui::SameLine();
        HelpMarker(FilenameExplanation.c_str());
        strhelpers::strip(m_extra);

        if (free_input()) {
            ImGui::Checkbox("Add datetime", &m_add_datetime);
        }
    }

    void increment()
    {
        if (!free_input()) {
            m_extra.clear();
            m_repeat_num += 1;
        }
    }

private:
    static constexpr const char *FilenameSuffix = ".dat";
    static constexpr unsigned int MaxParticipantId = 9999;

    enum class TestType {
        TidalBreathing,
        StrongCough,
        InducedCough,
        FreeInput,
    };

    struct TestTypeString {
        const char *long_name;
        const char *short_name;
    };

    static constexpr std::array<TestTypeString, magic_enum::enum_count<TestType>()>
        TestTypeStrings = {{
            {"Tidal breathing (TB)", "TB"},
            {"Strong cough (SC)", "SC"},
            {"Induced cough (IC)", "IC"},
            {"None (free input)", ""},
        }};

    [[nodiscard]] inline bool free_input() const { return m_test_type == TestType::FreeInput; }

    static bool test_type_combo(const char *label, TestType& test_type)
    {
        int index = magic_enum::enum_integer(test_type);
        const int old_index = index;
        ImGui::Combo(
            label,
            &index,
            [](void *, int idx) { return TestTypeStrings.at(idx).long_name; },
            nullptr,
            magic_enum::enum_count<TestType>()
        );
        test_type = magic_enum::enum_cast<TestType>(index).value();
        return index != old_index;
    }

    static void input_positive_int(const char *label, unsigned int& val)
    {
        int int_val = static_cast<int>(val);
        if (ImGui::InputInt(label, &int_val) && int_val > 0) {
            val = int_val;
        }
    }

    unsigned int m_participant_id = 1;
    TestType m_test_type = TestType::TidalBreathing;
    unsigned int m_repeat_num = 1;
    std::string m_extra;
    bool m_add_datetime = true;
};

class OutputFileController {
public:
    OutputFileController(const Device& device, DirectorySelector& dir_selector) :
        m_device(device), m_dir_selector(dir_selector)
    {}

    void draw()
    {
        using enum Device::State;
        const auto state = m_device.state();
        ImGui::SeparatorText(ICON_FA_FLOPPY_DISK "\tOutput");
        if (state == Recording || state == Stopping) {
            if (m_output_path.has_value()) {
                ImGui::TextWrapped("Output file path: %s", m_output_path->string().c_str());
            } else {
                ImGui::TextWrapped("Not writing to file");
            }
        } else {
            ImGui::Checkbox("Save to file", &m_write_output);
            if (m_write_output) {
                draw_output_section();
            } else {
                m_output_path.reset();
            }
        }
    }

    [[nodiscard]] const std::optional<std::filesystem::path>& output_path() const
    {
        return m_output_path;
    }

    [[nodiscard]] bool error() const { return m_write_output && !m_output_path.has_value(); }

    void increment_filename_data() { m_filename_form.increment(); }

private:
    void draw_output_section()
    {
        auto dir_path = m_dir_selector.path().string();
        bool dir_exists = m_dir_selector.exists();
        m_output_path.reset();

        if (dir_exists) {
            ImGui::TextWrapped("Output folder: %s", dir_path.c_str());
        } else {
            ImGui::TextWrapped("Folder does not exist: %s", dir_path.c_str());
        }

        if (ImGui::Button(ICON_FA_FOLDER "  Change folder...")) {
            m_dir_selector.select();
        }

        if (dir_exists) {
            m_filename_form.draw();
            const char *filename_error = m_filename_form.error();
            if (filename_error) {
                ImGui::TextWrapped("Error: %s", filename_error);
            } else {
                auto path = m_dir_selector.file_path(m_filename_form.filename());

                // Check if path is a directory to see if we can write to it. This is not a complete
                // check for writability of the path, but all other cases (e.g. no disk space,
                // incorrect permissions... which are unlikely) will be reported as an error when
                // attempting to open the path.
                if (std::filesystem::is_directory(path)) {
                    ImGui::Text("Error: path is a directory");
                } else {
                    if (std::filesystem::exists(path)) {
                        ImGui::TextWrapped("Warning: file already exists and will be overwritten");
                    }
                    m_output_path.emplace(path);
                }
            }
        }
    }

    const Device& m_device;
    DirectorySelector& m_dir_selector;
    RecordingFilenameForm m_filename_form;
    std::optional<std::filesystem::path> m_output_path;
    bool m_write_output = true;
};

class RecordTimeSelector {
public:
    explicit RecordTimeSelector(const Device& device) : m_device(device) {}

    void draw()
    {
        using enum Device::State;
        const auto state = m_device.state();
        ImGui::SeparatorText(ICON_FA_STOPWATCH "\tTimer");
        if (state == Recording || state == Stopping) {
            const CompleteSample::raw_millis_t millis_ = millis();
            if (millis_) {
                draw_timer_value(millis_);
            } else {
                ImGui::TextWrapped("Timer not set");
            }
        } else {
            ImGui::Checkbox("Run for set time", &m_enabled);
            if (m_enabled) {
                draw_timer_control();
            }
        }
    }

    bool error() const { return m_enabled && m_record_time == 0; }

    CompleteSample::raw_millis_t millis() const { return m_enabled ? m_record_time : 0; }

private:
    static void draw_timer_value(CompleteSample::raw_millis_t time_millis)
    {
        const CompleteSample::raw_millis_t total_secs = time_millis / 1000;
        const CompleteSample::raw_millis_t mins = total_secs / 60;
        const CompleteSample::raw_millis_t secs = total_secs % 60;
        if (mins && secs) {
            ImGui::TextWrapped(
                "Stopping after %u minute%s and %u second%s",
                mins,
                mins == 1 ? "" : "s",
                secs,
                secs == 1 ? "" : "s"
            );
        } else if (mins) {
            ImGui::TextWrapped("Stopping after %u minute%s", mins, mins == 1 ? "" : "s");
        } else {
            ImGui::TextWrapped("Stopping after %u second%s", secs, secs == 1 ? "" : "s");
        }
    }

    void draw_timer_control()
    {
        const auto time_width = ImGui::GetWindowSize().x / 4;
        const bool sameline = time_width > 90;

        ImGui::TextWrapped("Automatically stop recording after:");
        if (sameline) {
            ImGui::PushItemWidth(time_width);
        }
        ImGui::InputInt("minutes", &m_minutes);
        ImGui::SameLine();
        if (!sameline) {
            help_marker();
        }
        ImGui::InputInt("seconds", &m_seconds);
        if (sameline) {
            ImGui::PopItemWidth();
        }
        ImGui::SameLine();
        help_marker();

        m_record_time = 0;
        const int64_t millis_ = static_cast<int64_t>(1000)
            * (static_cast<int64_t>(m_seconds) + static_cast<int64_t>(m_minutes) * 60);
        if (millis_ <= 0) {
            ImGui::Text("Total time must be > 0!");
        } else if (millis_ >= std::numeric_limits<CompleteSample::raw_millis_t>::max()) {
            ImGui::Text("Time too long!");
        } else {
            m_record_time = static_cast<CompleteSample::raw_millis_t>(millis_);
        }
    }

    static inline void help_marker() { HelpMarker("Double-click to enter via keyboard"); }

    const Device& m_device;
    bool m_enabled = false;
    CompleteSample::raw_millis_t m_record_time = 0;
    int m_seconds = 0;
    int m_minutes = 0;
};

class ConnectedState : public Controller::State {
public:
    ConnectedState(
        Controller& controller, Device& device, const Device::ConnectedDeviceInfo& info
    ) :
        Controller::State(controller, device),
        m_port_name(info.port_name),
        m_version_string(info.version_str),
        m_build_digest(info.build_digest),
        m_writer_error_sub(subscribe_writer_error([this, info](const std::string& error) {
            set_state<WriterErrorState>(error, info);
        })),
        m_output_controller(device, m_dir_selector),
        m_record_time_selector(device)
    {}

    void update_and_draw() override
    {
        ImGui::Text("Device connected (%s)", m_port_name.c_str());
        ImGui::TextWrapped("Firmware version: %s", m_version_string.c_str());
        ImGui::TextWrapped("Firmware build ID: %s", m_build_digest.c_str());

        m_output_controller.draw();
        m_record_time_selector.draw();
        draw_record_control();
    }

private:
    [[nodiscard]] bool can_record() const
    {
        return !(m_output_controller.error() || m_record_time_selector.error());
    }

    void draw_record_control()
    {
        constexpr auto GrayColor = ImColor(128, 128, 128);
        constexpr float GreenHue = 120 / 360.0F;
        constexpr float RedHue = 0;

        using enum Device::State;
        ImGui::SeparatorText("##blank_record_control_separator");
        const auto state = m_device.state();
        if (state == Stopping) {
            ImGui::Text("Stopping%s", ellipsis_animated());
        } else {
            const bool cannot_record = state == Idle && !can_record();
            if (cannot_record) {
                ImGui::TextWrapped(ICON_FA_TRIANGLE_EXCLAMATION
                                   " Error(s) preventing recording! (See above)");
                ImGui::Spacing();
                PushButtonColor(GrayColor);
            } else {
                if (state == Idle && !m_output_controller.output_path()) {
                    ImGui::Text("Warning: not saving to file");
                    ImGui::Spacing();
                }
                PushButtonColorHSV(state == Idle ? GreenHue : RedHue);
            }

            ImGui::BeginDisabled(cannot_record);
            if (ImGui::Button(
                    state == Idle ? "Start" :
                                    (m_record_time_selector.millis() ? "Stop early" : "Stop")
                ))
            {
                toggle_recording();
            }
            ImGui::EndDisabled();
            PopButtonColor();
        }
    }

    void start_recording()
    {
        const auto& output_path = m_output_controller.output_path();
        if (output_path) {
            set_next_output_path(*output_path);
            m_output_controller.increment_filename_data();
        } else {
            clear_next_output_path();
        }
        m_device.start_recording(m_record_time_selector.millis());
    }

    void toggle_recording()
    {
        using enum Device::State;
        const auto state = m_device.state();
        if (state == Idle) {
            start_recording();
        } else if (state == Recording) {
            m_device.stop_recording();
        }
    }

    std::string m_port_name;
    std::string m_version_string;
    std::string m_build_digest;
    WriterObserver::ErrorObservable::Observer m_writer_error_sub;
    OutputFileController m_output_controller;
    RecordTimeSelector m_record_time_selector;
};

class WriterErrorState : public Controller::State {
public:
    WriterErrorState(
        Controller& controller,
        Device& device,
        const std::string& error,
        Device::ConnectedDeviceInfo info
    ) :
        Controller::State(controller, device),
        m_writer_error("Error writing to file: " + error),
        m_info(std::move(info))
    {}

    void update_and_draw() override
    {
        ImGui::TextWrapped("%s", m_writer_error.c_str());
        const auto state = m_device.state();
        if (state == Device::State::Stopping) {
            ImGui::Text("Stopping%s", ellipsis_animated());
        } else if (state == Device::State::Recording) {
            m_device.stop_recording();
        } else if (ImGui::Button("Ok")) {
            set_state<ConnectedState>(m_info);
        }
    }

private:
    std::string m_writer_error;
    Device::ConnectedDeviceInfo m_info;
};

class NotConnectedState : public Controller::State {
public:
    using Controller::State::State;

    NotConnectedState(Controller& controller, Device& device, std::string_view error) :
        Controller::State(controller, device), m_error(error)
    {}

    void update_and_draw() override
    {
        using enum Device::State;
        if (m_device.state() == Detecting) {
            draw_detecting();
        } else {
            draw_detect();
        }
    }

private:
    static void draw_detecting() { ImGui::Text("Detecting device%s", ellipsis_animated()); }

    void draw_detect()
    {
        if (m_error.empty()) {
            ImGui::TextWrapped("Cannot detect device");
        } else {
            ImGui::TextWrapped("Error: %s", m_error.c_str());
        }
        if (ImGui::Button("Detect device")) {
            m_device.detect_device();
        }
    }

    std::string m_error;
};

Controller::Controller(Device& device, WriterObserver& writer) :
    m_device(device),
    m_writer(writer),
    m_connect_sub(device.subscribe_connected([this](const auto& info) {
        set_state<ConnectedState>(info);
    })),
    m_disconnect_sub(device.subscribe_disconnected([this](const std::string *error) {
        if (error) {
            set_state<NotConnectedState>(*error);
        } else {
            set_state<NotConnectedState>();
        }
    }))
{
    set_state<NotConnectedState>();
}

Controller::~Controller() = default;

void Controller::update_and_draw()
{
    m_state->update_and_draw();
}

template <class S, class... Args> void Controller::set_state(Args&&...args)
{
    m_state = std::make_unique<S>(*this, m_device, std::forward<Args>(args)...);
}

}; // namespace recap::app::gui::controller
