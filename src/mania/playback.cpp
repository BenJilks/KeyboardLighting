#include "playback.hpp"
#include "osu_parser.hpp"
#include <set>

using namespace Mania;

void Mania::apply_note_step(std::vector<Note>& notes, float step)
{
    for (auto& note : notes) {
        note.column -= step;
    }
}

std::vector<Note> Mania::build_note_sequence(Osu const& osu)
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
