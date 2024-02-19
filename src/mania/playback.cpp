#include "playback.hpp"
#include "osu_parser.hpp"
#include <math.h>
#include <set>

using namespace Mania;

enum class Score {
    PERFECT,
    GOOD,
    OK,
};

GameState Mania::initialize_game_state(Osu const& osu)
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
            .time = hit_object.time / 1000.0f,
            .length = length,
            .color = colors[row],
        });
    }

    return GameState {
        .notes = notes,
    };
}

static std::optional<Score> get_score(float distance)
{
    if (distance <= 0.01) {
        return Score::PERFECT;
    } else if (distance <= 0.05) {
        return Score::GOOD;
    } else if (distance <= 0.1) {
        return Score::OK;
    } else {
        return std::nullopt;
    }
}

static LedKeyboard::Color color_for_score(Score score)
{
    switch (score) {
    case Score::PERFECT:
        return { .green = 0xFF };
    case Score::GOOD:
        return { .red = 0xFF, .green = 0xFF };
    case Score::OK:
        return { .red = 0xFF, .green = 0x7F };
    }
}

static float health_for_score(Score score)
{
    switch (score) {
    case Score::PERFECT:
        return 0.1;
    case Score::GOOD:
        return 0.05;
    case Score::OK:
        return 0.01;
    }
}

static void clamp_health(GameState &state)
{
    if (state.health > 1) {
        state.health = 1;
    } else if (state.health < 0) {
        state.health = 0;
    }
}

void Mania::on_key_pressed(GameState& state, uint row)
{
    for (auto& note : state.notes) {
        if (note.hit) {
            continue;
        }

        float distance = std::fabs(note.time - state.time);
        auto score_or_none = get_score(distance);
        if (!score_or_none) {
            continue;
        }

        // On note hit.
        auto score = score_or_none.value();
        state.hit_indicator = color_for_score(score);
        state.health += health_for_score(score);
        note.hit = true;
    }

    clamp_health(state);
}

void Mania::register_misses(GameState &state)
{
    for (auto& note : state.notes) {
        if (note.hit) {
            continue;
        }

        if (state.time - note.time <= 0.1) {
            continue;
        }

        // On note miss.
        state.hit_indicator = { .red = 0xFF };
        state.health -= 0.2;
        note.hit = true;
    }

    clamp_health(state);
}
