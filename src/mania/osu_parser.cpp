#include "osu_parser.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <format>
#include <charconv>
#include <algorithm>

using namespace Mania;

enum class State {
    Initial,
    BeforeFirstCategory,
    Entry,
};

enum class Category {
    None,
    General,
    Editor,
    Metadata,
    Difficulty,
    Events,
    TimingPoints,
    HitObjects,
};

static std::string parse_version_line(std::string_view line) {
    // TODO: Implement this.
    return std::string(line);
}

static Category parse_category(std::string_view line) {
    size_t start = 0;
    size_t end = line.size() - 1;
    while (start < line.size() - 1 && line[start] != '[') { start += 1; }
    while (end > 1 && line[end] != ']') { end -= 1; }

    auto category_name = line.substr(start + 1, end - start - 1);
    if (category_name == "General") {
        return Category::General;
    } else if (category_name == "Editor") {
        return Category::Editor;
    } else if (category_name == "Metadata") {
        return Category::Metadata;
    } else if (category_name == "Difficulty") {
        return Category::Difficulty;
    } else if (category_name == "Events") {
        return Category::Events;
    } else if (category_name == "TimingPoints") {
        return Category::TimingPoints;
    } else if (category_name == "HitObjects") {
        return Category::HitObjects;
    } else {
        std::cout << "Unrecognised category name '" << category_name << "'\n";
        return Category::None;
    }
}

static std::pair<std::string, std::string> parse_key_value_pair(std::string_view line) {
    enum State {
        Key,
        BeforeValue,
        Value,
    };

    std::string key;
    std::string value;
    State state = Key;
    for (char c : line) {
        switch (state) {
            case Key:
                if (c == ':') {
                    state = BeforeValue;
                } else {
                    key += c;
                }
                break;
            
            case BeforeValue:
                if (!isspace(c)) {
                    state = Value;
                    value += c;
                }
                break;
            
            case Value:
                value += c;
                break;
        }
    }

    return std::make_pair(key, value);
}

static std::vector<std::string_view> parse_separated_values(std::string_view line, char separator) {
    std::vector<std::string_view> output;

    size_t start = 0;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == separator) {
            output.push_back(line.substr(start, i - start));
            start = i + 1;
        }
    }

    output.push_back(line.substr(start, line.size() - start - 1));
    return output;
}

static TimingPoint parse_timing_point(std::string_view line) {
    auto values = parse_separated_values(line, ',');

    auto parse_int = [](std::string_view value) {
        int i = 0;
        std::from_chars(value.data(), value.data() + value.size(), i);
        return i;
    };

    auto parse_float = [](std::string_view value) {
        float f = 0;
        std::from_chars(value.data(), value.data() + value.size(), f);
        return f;
    };

    return TimingPoint {
        .time = parse_int(values[0]),
        .beat_length = parse_float(values[1]),
        .meter = parse_int(values[2]),
        .sample_set = parse_int(values[3]),
        .sample_index = parse_int(values[4]),
        .volume = parse_int(values[5]),
        .uninherited = parse_int(values[6]),
    };
}

static std::array<int, 5> parse_params(std::string_view param_str) {
    std::array<int, 5> params {};

    auto values = parse_separated_values(param_str, ':');
    for (size_t i = 0; i < std::min(values.size(), 5ul); ++i) {
        const auto &value_str = values.at(i);

        int value = 0;
        std::from_chars(value_str.data(), value_str.data() + value_str.size(), value);
        params[i] = value;
    }

    return params;
}

static HitObject parse_hit_object(std::string_view line) {
    auto values = parse_separated_values(line, ',');

    auto parse_int = [](std::string_view value) {
        int i = 0;
        std::from_chars(value.data(), value.data() + value.size(), i);
        return i;
    };

    return HitObject {
        .x = parse_int(values[0]),
        .y = parse_int(values[1]),
        .time = parse_int(values[2]),
        .type = parse_int(values[3]),
        .hit_sound = parse_int(values[4]),
        .params = parse_params(values[5]),
    };
}

static void parse_entry(Osu &osu,
                        std::string_view line,
                        Category current_category) {
    switch (current_category) {
        case Category::None:
            // NOTE: We shouldn't be here, but we've already reported this error.
            break;

        case Category::General:
            osu.general.insert(parse_key_value_pair(line));
            break;

        case Category::Editor:
            osu.editor.insert(parse_key_value_pair(line));
            break;

        case Category::Metadata:
            osu.metadata.insert(parse_key_value_pair(line));
            break;

        case Category::Difficulty:
            osu.difficulty.insert(parse_key_value_pair(line));
            break;

        case Category::Events:
            // NOTE: We don't parse events for the moment.
            break;

        case Category::TimingPoints:
            osu.timing_points.push_back(parse_timing_point(line));
            break;

        case Category::HitObjects:
            osu.hit_objects.push_back(parse_hit_object(line));
            break;
    }
}

Osu Mania::parse_osu_file(std::string_view file_path)
{
    auto parsed_file = Osu{};
    auto state = State::Initial;
    auto current_category = Category::None;

    auto file = std::ifstream(std::string(file_path));
    std::string line;
    while (std::getline(file, line)) {
        switch (state) {
            case State::Initial: {
                if (line.rfind("osu", 0) == 0) {
                    parsed_file.version = parse_version_line(line);
                    state = State::BeforeFirstCategory;
                }

                break;
            }

            case State::BeforeFirstCategory: {
                if (line.length() > 0 && line[0] == '[') {
                    current_category = parse_category(line);
                    state = State::Entry;
                }

                break;
            }

            case State::Entry: {
                // Skip empty lines
                if (line.length() == 0 || std::isspace(line[0])) {
                    break;
                }

                // Change category
                if (line.length() > 0 && line[0] == '[') {
                    current_category = parse_category(line);
                    break;
                }

                parse_entry(parsed_file, line, current_category);
                break;
            }
        }
    }

    return parsed_file;
}
