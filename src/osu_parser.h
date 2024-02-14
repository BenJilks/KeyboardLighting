#include <string>
#include <map>
#include <vector>

struct TimingPoint {
    int time;
    float beat_length;
    int meter;
    int sample_set;
    int sample_index;
    int volume;
    int uninherited;
};

struct HitObject {
    int x;
    int y;
    int time;
    int type;
};

struct Osu {
    std::string version;
    std::map<std::string, std::string> general;
    std::map<std::string, std::string> editor;
    std::map<std::string, std::string> metadata;
    std::map<std::string, std::string> difficulty;
    std::vector<TimingPoint> timing_points;
    std::vector<HitObject> hit_objects;
};

Osu parse_osu_file(std::string_view file_path);
