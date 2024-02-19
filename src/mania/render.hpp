#include "forward.hpp"
#include "keyboard.hpp"
#include <vector>

namespace Mania {

void render_frame(
    LedKeyboard& keyboard,
    std::vector<Note> const& notes,
    LedKeyboard::Color hit_indicator,
    float health,
    float time);

}
