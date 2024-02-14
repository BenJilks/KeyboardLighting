#include "keyboard.h"
#include "keyboard_layout.h"
#include "osu_parser.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <time.h>
#include <unistd.h>

struct Note {
    int row;
    int column;
    uint length;
};

void render_note(std::vector<LedKeyboard::KeyValue>& keys, Note const& note)
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
            .color = { .red = 0xFF, .green = 0xFF, .blue = 0xFF },
        });
    }
}

void render_frame(LedKeyboard& keyboard, std::vector<Note> const& notes)
{
    std::vector<LedKeyboard::KeyValue> keys;
    for (auto const& note : notes) {
        render_note(keys, note);
    }

    keyboard.setAllKeys({ .red = 0, .green = 0, .blue = 0 });
    keyboard.setKeys(keys);
    keyboard.commit();
}

void apply_note_step(std::vector<Note>& notes)
{
    for (auto& note : notes) {
        note.column -= 1;
    }
}

std::vector<Note> build_note_sequence(Osu const& osu)
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
            .column = hit_object.time / 100,
            .length = length,
        });
    }

    return notes;
}

void play_song(LedKeyboard& keyboard)
{
    auto osu = parse_osu_file("test.osu");
    auto notes = build_note_sequence(osu);

    for (;;) {
        render_frame(keyboard, notes);
        apply_note_step(notes);

        usleep(100 * 1000);
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
