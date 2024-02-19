#include "keyboard_layout.hpp"
#include "render.hpp"
#include "playback.hpp"

using namespace Mania;

static void render_note(
    std::vector<LedKeyboard::KeyValue>& keys,
    Note const& note,
    float time)
{
    const auto start_column = static_cast<int>((note.time - time) * 10);
    for (int i = 0; i < note.length; ++i) {
        auto column = start_column + i;
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

void Mania::render_frame(
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
        render_note(keys, note, time);
    }

    keyboard.setAllKeys({ .red = 0, .green = 0, .blue = 0 });
    keyboard.setKeys(keys);
    keyboard.commit();
}
