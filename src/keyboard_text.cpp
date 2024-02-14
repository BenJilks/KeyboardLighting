#include "keyboard.h"
#include "keyboard_layout.h"
#include <cctype>
#include <string_view>
#include <unistd.h>

const char *font[] =
{
"  X  |XXXX |  XXX|XXXX |XXX  |XXX  |XXXX |X  X |XXX  |  X  |X  X |X    |X   X|X   X|XXXXX|XXXX | XXX |XXX  | XXX |XXXXX|X   X|X   X|X   X|X   X|X   X|XXXXX",
" X X |X   X| X   |X   X|X    |X    |X    |X  X | X   |  X  |X X  |X    |XX XX|XX  X|X   X|X   X|X   X|X  X |X    |  X  |X   X|X   X|X   X| X X | X X |   X ",
"X   X|XXXX |X    |X   X|XXX  |XXX  |X XX |XXXX | X   |  X  |XX   |X    |X X X|X X X|X   X|XXXX |X X X|XXX  | XX  |  X  |X   X|X   X|X X X|  X  |  X  |  X  ",
"XXXXX|X   X| X   |X   X|X    |X    |X  X |X  X | X   |  X  |X X  |X    |X   X|X  XX|X   X|X    |X  XX|X  X |   X |  X  |X   X| X X |XX XX| X X |  X  | X   ",
"X   X|XXXX |  XXX|XXXX |XXX  |X    |XXXX |X  X |XXX  |XXX  |X  X |XXX  |X   X|X   X|XXXXX|X    | XXX |X   X|XXX  |  X  | XXX |  X  |X   X|X   X|  X  |XXXXX",
};

auto background_color = LedKeyboard::Color { (int)(0x00 * 0.4), (int)(0x8f * 0.4), (int)(0xd5 * 0.4) };
auto foreground_color = LedKeyboard::Color { 0xfb, 0xab, 0x18 };

static void show_char(LedKeyboard &keyboard, char c, int pos_x, int pos_y, LedKeyboard::Color color)
{
    c = std::toupper(c);

    if (c < 'A' || c > 'Z')
        return;

    std::vector<LedKeyboard::KeyValue> keys;
    int index = (c - 'A') * 6;
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (pos_x + x < 0 || pos_x + x > Layout::map[0].size())
                continue;

            auto font_char = font[y][index + x];
            if (font_char == ' ')
                continue;

            auto key = Layout::map[pos_y + y][pos_x + x];
            if (!key)
                continue;

            keys.push_back(LedKeyboard::KeyValue { *key, color });
        }
    }

    if (!keys.empty())
        keyboard.setKeys(keys);
}

static void scroll_text(LedKeyboard &keyboard, std::string_view text)
{
    int offset = Layout::map[0].size();
    while (offset > -((int)text.size() * 5))
    {
        keyboard.setAllKeys(background_color);
        for (int i = 0; i < text.size(); i++)
        {
            int char_offset = offset + (i * 5);
            show_char(keyboard, text[i], char_offset, 1, foreground_color);
        }
        
        keyboard.commit();
        usleep(1000 * 100);
        offset -= 1;
    }
}

int main()
{
    if (getuid() != 0)
    {
        std::cerr << "Error: Can be only run as root\n";
        return 1;
    }

    LedKeyboard keyboard;
    if (!keyboard.open())
    {
        std::cerr << "Error: Unable to connect to keyboard\n";
        return 1;
    }

    scroll_text(keyboard, "Hello world");
    keyboard.close();
    return 0;
}

