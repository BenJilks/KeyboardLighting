#include <sys/types.h>
#include "forward.hpp"
#include "keyboard.hpp"

namespace Mania {

struct GameState {
    std::vector<Note> notes;
    LedKeyboard::Color hit_indicator { .green = 0xFF };
    float health { 0.7 };
    float time { 0 };
};

struct Note {
    int row;
    float time;
    uint length;
    LedKeyboard::Color color;

    bool hit { false };
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

GameState initialize_game_state(Osu const& osu);

void on_key_pressed(GameState &state, uint row);
void register_misses(GameState &state);

}
