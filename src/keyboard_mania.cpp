#include "keyboard.h"
#include "keyboard_layout.h"
#include "osu_parser.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <time.h>
#include <unistd.h>
#include <math.h>

struct Note {
    int row;
    float column;
    uint length;

    LedKeyboard::Color color;
};

static auto const colors = std::array {
    LedKeyboard::Color { .red = 0xff, .green = 0x80, .blue = 0xed },
    LedKeyboard::Color { .red = 0x06, .green = 0x55, .blue = 0x35 },
    LedKeyboard::Color { .red = 0xff, .green = 0xc0, .blue = 0xcb },
    LedKeyboard::Color { .red = 0xff, .green = 0xff, .blue = 0xff },
    LedKeyboard::Color { .red = 0xff, .green = 0xe4, .blue = 0xe1 },
    LedKeyboard::Color { .red = 0x00, .green = 0x80, .blue = 0x80 },
    LedKeyboard::Color { .red = 0x13, .green = 0x33, .blue = 0x37 },
};

static void render_note(
    std::vector<LedKeyboard::KeyValue>& keys,
    Note const& note)
{
    for (int i = 0; i < note.length; ++i) {
        auto column = note.column + i;
        if (column < 0 || column >= 13) {
            continue;
        }

        auto key = Layout::map[note.row + 1][column];
        if (!key) {
            continue;
        }

        keys.push_back(LedKeyboard::KeyValue {
            .key = *key,
            .color = note.color,
        });
    }
}

static void render_health_bar(
    std::vector<LedKeyboard::KeyValue>& keys,
    float health)
{
    for (int i = 0; i < health * 20; i++) {
        keys.push_back(LedKeyboard::KeyValue {
            .key = *Layout::map[0][i],
            .color = {
                .red = static_cast<uint8_t>(0xFF * (1.0 - health)),
                .green = static_cast<uint8_t>(0xFF * health),
                .blue = 0,
            },
        });
    }
}

static void render_hit_indicator(
    std::vector<LedKeyboard::KeyValue>& keys,
    LedKeyboard::Color hit_indicator)
{
    const auto start_column = 20;
    const auto end_column = 24;

    for (int row = 0; row < Layout::map.size(); ++row) {
        for (int column = start_column; column < end_column; ++column) {
            auto key = Layout::map[row][column];
            if (!key) {
                continue;
            }

            keys.push_back(LedKeyboard::KeyValue {
                .key = *key,
                .color = hit_indicator,
            });
        }
    }
}

static void render_frame(
    LedKeyboard& keyboard,
    std::vector<Note> const& notes,
    LedKeyboard::Color hit_indicator,
    float health,
    float time)
{
    std::vector<LedKeyboard::KeyValue> keys;
    render_health_bar(keys, health);
    render_hit_indicator(keys, hit_indicator);
    for (auto const& note : notes) {
        render_note(keys, note);
    }

    keyboard.setAllKeys({ .red = 0, .green = 0, .blue = 0 });
    keyboard.setKeys(keys);
    keyboard.commit();
}

static void apply_note_step(std::vector<Note>& notes, float step)
{
    for (auto& note : notes) {
        note.column -= step;
    }
}

static std::vector<Note> build_note_sequence(Osu const& osu)
{
    std::set<int> x_positions;
    for (auto const& hit_object : osu.hit_objects) {
        x_positions.insert(hit_object.x);
    }

    auto x_to_row = std::vector(x_positions.begin(), x_positions.end());
    std::sort(x_to_row.begin(), x_to_row.end());

    std::vector<Note> notes;
    for (auto const& hit_object : osu.hit_objects) {
        auto it = std::find(x_to_row.begin(), x_to_row.end(), hit_object.x);
        int row = it - x_to_row.begin();
        if (row > 3) {
            continue;
        }

        uint length = 1;
        if (hit_object.type & 128) {
            length = (hit_object.params[0] - hit_object.time) / 100;
        }

        notes.push_back(Note {
            .row = row,
            .column = static_cast<float>(hit_object.time / 100),
            .length = length,
            .color = colors[row],
        });
    }

    return notes;
}

static void play_song(LedKeyboard& keyboard)
{
    auto osu = parse_osu_file("test.osu");
    auto notes = build_note_sequence(osu);

    float time = 0;
    float health = 0.7;
    LedKeyboard::Color hit_indicator = { .green = 0xFF };

    for (;;) {
        render_frame(keyboard, notes, hit_indicator, health, time);

        apply_note_step(notes, 1);
        usleep(100 * 1000);
        time += 0.1f;
    }
}

int main()
{
    if (getuid() != 0) {
        std::cerr << "Error: Can be only run as root\n";
        return 1;
    }

    LedKeyboard keyboard;
    if (!keyboard.open()) {
        std::cerr << "Error: Unable to connect to keyboard\n";
        return 1;
    }

    play_song(keyboard);
    keyboard.close();
    return 0;
}
