#pragma once

#include "keyboard.h"
#include <optional>
#include <vector>

namespace Layout {

using enum LedKeyboard::Key;
auto blank = std::nullopt;

std::vector<std::vector<std::optional<LedKeyboard::Key>>> map = {
    { esc, f1, f2, f3, f4, blank, f5, f6, f7, f8, blank, f9, f10, f11, f12, blank, print_screen, scroll_lock, pause_break, blank, blank, blank, blank, blank },
    { tilde, n1, n2, n3, n4, n5, n6, n7, n8, n9, n0, minus, equal, backspace, blank, blank, insert, home, page_up, blank, num_lock, num_slash, num_asterisk, num_minus },
    { tab, q, w, e, r, t, y, u, i, o, p, open_bracket, close_bracket, blank, enter, blank, del, end, page_down, blank, num_7, num_8, num_9, num_plus },
    { caps_lock, a, s, d, f, g, h, j, k, l, semicolon, quote, blank, blank, blank, blank, blank, blank, blank, blank, num_4, num_5, num_6, blank },
    { shift_left, backslash, z, x, c, v, b, n, m, comma, period, slash, blank, shift_right, blank, blank, blank, arrow_top, blank, blank, num_1, num_2, num_3, num_enter },
    { ctrl_left, win_left, alt_left, blank, blank, blank, space, blank, blank, blank, alt_right, blank, blank, menu, ctrl_right, blank, arrow_left, arrow_bottom, arrow_right, blank, num_0, blank, num_dot, blank },
};

}
