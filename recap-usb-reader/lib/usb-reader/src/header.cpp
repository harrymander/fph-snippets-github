#include "record.hpp"

#include <charconv>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace std::string_literals;

namespace recap::record {

InvalidRecordHeaderError InvalidRecordHeaderError::from_invalid_line(std::string_view line)
{
    std::ostringstream ss;
    ss << "invalid header line: " << line;
    return InvalidRecordHeaderError(ss.str());
}

InvalidRecordHeaderError InvalidRecordHeaderError::from_missing_key(std::string_view key)
{
    std::ostringstream ss;
    ss << "missing header field: " << key;
    return InvalidRecordHeaderError(ss.str());
}

InvalidRecordHeaderError
InvalidRecordHeaderError::from_invalid_numeric(std::string_view numeric, std::errc err)
{
    std::ostringstream ss;
    ss << "invalid numeric value in header";
    if (err != std::errc{}) {
        ss << " (" << std::make_error_code(err).message() << ')';
    }
    ss << ": " << numeric;
    return InvalidRecordHeaderError(ss.str());
}

static Header::FieldsMap::value_type split_header_line(const std::string& line)
{
    auto pos = line.find(':');
    if (pos == 0 || pos == std::string::npos) {
        throw InvalidRecordHeaderError::from_invalid_line(line);
    }
    const auto key = line.substr(0, pos);
    pos = line.find_first_not_of(' ', pos + 1);
    if (pos == std::string::npos) {
        throw InvalidRecordHeaderError::from_invalid_line(line);
    }
    return {key, line.substr(pos)};
}

static Header::FieldsMap parse_header_map(const std::string& buffer)
{
    Header::FieldsMap map;
    std::istringstream ss(buffer);
    for (std::string line; std::getline(ss, line);) {
        map.insert(split_header_line(line));
    }
    return map;
}

static Header::FieldsMap::mapped_type
pop_from_header(Header::FieldsMap& map, const std::string& key)
{
    const auto node = map.extract(key);
    if (node.empty()) {
        throw InvalidRecordHeaderError::from_missing_key(key);
    }
    return std::move(node.mapped());
}

template <class T> static T parse_integer(std::string_view val)
{
    T number;
    const auto end = val.data() + val.size();
    const auto res = std::from_chars(val.data(), end, number, 10);
    if (res.ptr != end || res.ec != std::errc{}) {
        throw InvalidRecordHeaderError::from_invalid_numeric(val, res.ec);
    }
    return number;
}

Header Header::from_buffer(const std::string& buffer)
{
    auto map = parse_header_map(buffer);
    return Header(
        FirmwareVersion(pop_from_header(map, "Firmware")),
        pop_from_header(map, "Build"),
        parse_integer<unsigned int>(pop_from_header(map, "Sample-Rate")),
        parse_integer<int>(pop_from_header(map, "VREF")),
        pop_from_header(map, "Board"),
        pop_from_header(map, "Flow"),
        parse_integer<int>(pop_from_header(map, "Flow-Offset")),
        parse_integer<int>(pop_from_header(map, "Flow-Scale")),
        parse_integer<uint8_t>(pop_from_header(map, "Flow-Sign")),
        pop_from_header(map, "H2S-Barcode"),
        map
    );
}

static std::string strip_and_check_str(const std::string& str)
{
    const auto pos = str.find_first_not_of(' ');
    if (pos == std::string::npos) {
        throw std::invalid_argument("empty string");
    }
    return str.substr(pos);
}

Header::Header(
    const FirmwareVersion& firmware_version,
    const std::string& build_digest,
    unsigned int sample_rate,
    int adc_vref_millivolts,
    const std::string& board_name,
    const std::string& flow_meter_model,
    int flow_meter_offset,
    int flow_meter_scale,
    bool flow_meter_signed,
    const std::string& h2s_sensor_barcode,
    const Header::FieldsMap& extra_fields
) :
    m_firmware_version(firmware_version),
    m_build_digest(strip_and_check_str(build_digest)),
    m_sample_rate(sample_rate),
    m_adc_vref(adc_vref_millivolts),
    m_board_name(strip_and_check_str(board_name)),
    m_flow_model(strip_and_check_str(flow_meter_model)),
    m_flow_offset(flow_meter_offset),
    m_flow_scale(flow_meter_scale),
    m_flow_signed(flow_meter_signed),
    m_h2s_sensor_barcode(strip_and_check_str(h2s_sensor_barcode)),
    m_extras(extra_fields)
{
    for (auto& [key, val] : m_extras) {
        if (key.empty()) {
            throw std::invalid_argument("empty field key");
        }
        const auto pos = val.find_first_not_of(' ');
        if (pos == std::string::npos) {
            throw std::invalid_argument("empty value for key: "s + key);
        }
        val = val.substr(pos);
    }

    if (m_sample_rate == 0) {
        throw std::invalid_argument("sample rate must be >0");
    }
}

std::string Header::buffer_str() const
{
    std::ostringstream ss;
    ss << "Firmware: " << firmware_version().str() << "\n"
       << "Build: " << build_digest() << "\n"
       << "Sample-Rate: " << sample_rate_khz() << "\n"
       << "VREF: " << adc_vref_millivolts() << "\n"
       << "Board: " << board_name() << "\n"
       << "Flow: " << flow_meter_model() << "\n"
       << "Flow-Offset: " << flow_meter_offset() << "\n"
       << "Flow-Scale: " << flow_meter_scale() << "\n"
       << "Flow-Sign: " << (flow_meter_signed() ? '1' : '0') << "\n"
       << "H2S-Barcode: " << h2s_sensor_barcode();

    for (const auto& [key, val] : m_extras) {
        ss << '\n' << key << ": " << val;
    }

    return ss.str();
}

const FirmwareVersion& Header::firmware_version() const
{
    return m_firmware_version;
}

const std::string& Header::build_digest() const
{
    return m_build_digest;
}

unsigned int Header::sample_rate_khz() const
{
    return m_sample_rate;
}

int Header::adc_vref_millivolts() const
{
    return m_adc_vref;
}

const std::string& Header::board_name() const
{
    return m_board_name;
}

const std::string& Header::flow_meter_model() const
{
    return m_flow_model;
}

int Header::flow_meter_offset() const
{
    return m_flow_offset;
}

int Header::flow_meter_scale() const
{
    return m_flow_scale;
}

bool Header::flow_meter_signed() const
{
    return m_flow_signed;
}

const std::string& Header::h2s_sensor_barcode() const
{
    return m_h2s_sensor_barcode;
}

Header::FieldsMap::size_type Header::num_extra_fields() const
{
    return m_extras.size();
}

const Header::FieldsMap& Header::extra_fields() const
{
    return m_extras;
}

const std::string *Header::get_extra_field(const std::string& key) const
{
    const auto iter = m_extras.find(key);
    return iter == m_extras.end() ? nullptr : &std::get<1>(*iter);
}

void Header::set_extra_field(const std::string& key, const std::string& value)
{
    m_extras[key] = value;
}

}; // namespace recap::record
