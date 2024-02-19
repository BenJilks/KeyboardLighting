#include <sys/types.h>
#include "forward.h"
#include "keyboard.h"

namespace Mania {

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

std::vector<Note> build_note_sequence(Osu const& osu);

void apply_note_step(std::vector<Note>& notes, float step);

}
