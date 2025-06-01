#include "gcode_gen/waypoint.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <vector>
#include <cctype>

namespace gcode_gen {
namespace {

static bool startsWith(const std::string& s, char c) {
    for (char ch : s) {
        if (!std::isspace(static_cast<unsigned char>(ch)))
            return ch == c;
    }
    return false;
}

static std::vector<std::string> split(const std::string& line, char delim) {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream ss(line);
    while (std::getline(ss, cur, delim)) {
        out.push_back(cur);
    }
    return out;
}

static bool parseBool(const std::string& s) {
    return s == "1" || s == "true" || s == "True";
}

} // namespace

std::vector<Waypoint> readWaypoints(const std::string& path, const std::string& z_field) {
    std::ifstream in(path);
    if (!in.is_open())
        throw std::runtime_error("Failed to open waypoint file: " + path);

    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    std::vector<Waypoint> waypoints;

    if (startsWith(contents, '[')) {
        // very small JSON parser for expected format
        size_t pos = 0;
        while (true) {
            pos = contents.find('{', pos);
            if (pos == std::string::npos) break;
            size_t end = contents.find('}', pos);
            if (end == std::string::npos) break;
            std::string obj = contents.substr(pos + 1, end - pos - 1);
            pos = end + 1;
            Waypoint wp;
            std::istringstream os(obj);
            std::string kv;
            while (std::getline(os, kv, ',')) {
                auto colon = kv.find(':');
                if (colon == std::string::npos) continue;
                std::string key = kv.substr(0, colon);
                std::string value = kv.substr(colon + 1);
                // trim spaces and quotes
                auto trim = [](std::string s) {
                    size_t b = 0;
                    while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
                    size_t e = s.size();
                    while (e > b && std::isspace(static_cast<unsigned char>(s[e-1]))) --e;
                    if (e > b && s[b] == '"') ++b;
                    if (e > b && s[e-1] == '"') --e;
                    return s.substr(b, e-b);
                };
                key = trim(key);
                value = trim(value);
                if (key == "lat") wp.lat = std::stod(value);
                else if (key == "lon") wp.lon = std::stod(value);
                else if (key == z_field) wp.alt = std::stod(value);
                else if (key == "tool") { wp.tool_down = parseBool(value); wp.has_tool = true; }
            }
            waypoints.push_back(wp);
        }
    } else {
        // CSV or TSV
        std::istringstream ss(contents);
        std::string header;
        if (!std::getline(ss, header))
            return waypoints;
        char delim = header.find('\t') != std::string::npos ? '\t' : ',';
        auto cols = split(header, delim);
        int lat_idx=-1, lon_idx=-1, alt_idx=-1, tool_idx=-1;
        for (size_t i=0;i<cols.size();++i) {
            if (cols[i]=="lat") lat_idx=i;
            else if (cols[i]=="lon") lon_idx=i;
            else if (cols[i]==z_field) alt_idx=i;
            else if (cols[i]=="tool") tool_idx=i;
        }
        if (lat_idx<0 || lon_idx<0)
            throw std::runtime_error("lat/lon columns required");
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            auto vals = split(line, delim);
            if ((int)vals.size() <= std::max({lat_idx, lon_idx, alt_idx, tool_idx})) continue;
            Waypoint wp;
            wp.lat = std::stod(vals[lat_idx]);
            wp.lon = std::stod(vals[lon_idx]);
            if (alt_idx>=0) wp.alt = std::stod(vals[alt_idx]);
            if (tool_idx>=0) { wp.tool_down = parseBool(vals[tool_idx]); wp.has_tool = true; }
            waypoints.push_back(wp);
        }
    }

    return waypoints;
}

} // namespace gcode_gen
