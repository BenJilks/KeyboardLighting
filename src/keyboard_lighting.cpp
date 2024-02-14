#include "Keyboard.h"
#include "utils.h"
#include <cstdio>
#include <vector>
#include <array>
#include <string_view>
#include <unordered_set>
#include <unistd.h>
#include <termios.h>

auto background_color = LedKeyboard::Color { (int)(0x00 * 0.4), (int)(0x8f * 0.4), (int)(0xd5 * 0.4) };
auto foreground_color = LedKeyboard::Color { 0xfb, 0xab, 0x18 };

std::vector<std::string_view> hour_digit_1[] = 
{
    { "3", "2", "4", "r", "f", "c", "x", "z", "a", "q" },
    { "3", "e", "d", "x", "f2" },
    { "f1", "f2", "f3", "f4", "5", "r", "e", "w", "a", "z", "x", "c" },
    { "f1", "f2", "f3", "f4", "5", "r", "e", "w", "f", "c", "x", "z" },
    { "f1", "2", "w", "e", "r", "5", "f4", "f", "c" },
    { "f1", "f2", "f3", "f4", "2", "r", "e", "w", "f", "z", "x", "c" },
    { "f1", "f2", "f3", "f4", "2", "r", "e", "w", "f", "z", "x", "c", "a", "q" },
    { "f1", "f2", "f3", "f4", "4", "e", "s", "z" },
    { "f1", "f2", "f3", "f4", "2", "5", "r", "f", "v", "c", "x", "z", "a", "q", "w", "e" },
    { "f1", "f2", "f3", "f4", "2", "5", "r", "f", "z", "c", "x", "w", "e" },
};

std::vector<std::string_view> hour_digit_2[] = 
{
    { "n", "h", "o", ",", "m", "9", "l", "y", "8", "7" },
    { "k", "8", "i", "m", "f6" },
    { ",", "h", "m", "u", "n", "i", "o", "0", "f5", "f6", "f8", "f7" },
    { ",", "m", "u", "n", "i", "o", "0", "f8", "l", "f7", "f6" },
    { "l", ",", "0", "o", "u", "i", "7", "f8", "f5" },
    { ",", "o", "n", "i", "m", "u", "f6", "l", "f7", "7", "f8", "f5" },
    { "o", ",", "i", "n", "h", "u", "m", "f6", "y", "l", "f7", "7", "f8" },
    { "n", "i", "f8", "j", "f7", "9", "f6", "f5" },
    { "f7", "l", "y", "f8", "7", "0", "o", "m", "n", "i", "u", "f6", "h", "," },
    { ",", "o", "n", "i", "m", "u", "0", "f6", "l", "f7", "7", "f8" },
};

static void set_digit(LedKeyboard &keyboard, const std::vector<std::string_view> &digit, const LedKeyboard::Color &color)
{
    std::vector<LedKeyboard::KeyValue> keys;
    for (const auto c : digit)
    {
        LedKeyboard::Key key;
        utils::parseKey(std::string(c), key);

        keys.push_back(LedKeyboard::KeyValue
        {
            .key = key,
            .color = color,
        });
    }

    keyboard.setKeys(keys);
}

static void morse_code_number(LedKeyboard &keyboard, int n)
{
    std::vector<LedKeyboard::KeyValue> keys;
    auto add_key = [&](auto ...new_keys)
    {
        for (auto key : { new_keys... })
        {
            keys.push_back(LedKeyboard::KeyValue
            {
                .key = key,
                .color = foreground_color,
            });
        }
    };

    if (n / 5 == 1)       add_key(LedKeyboard::Key::num_asterisk);
    else                  add_key(LedKeyboard::Key::num, LedKeyboard::Key::num_slash);

    if ((n + 1) / 5 == 1) add_key(LedKeyboard::Key::num_7, LedKeyboard::Key::num_8);
    else                  add_key(LedKeyboard::Key::num_9);

    if ((n + 2) / 5 == 1) add_key(LedKeyboard::Key::num_4, LedKeyboard::Key::num_5);
    else                  add_key(LedKeyboard::Key::num_6);

    if ((n + 3) / 5 == 1) add_key(LedKeyboard::Key::num_1, LedKeyboard::Key::num_2);
    else                  add_key(LedKeyboard::Key::num_3);

    if ((n + 4) / 5 == 1) add_key(LedKeyboard::Key::num_0);
    else                  add_key(LedKeyboard::Key::num_dot);

    keyboard.setKeys(keys);
}

static void direction_number(LedKeyboard &keyboard, int n)
{
    std::vector<LedKeyboard::Key> directions[] =
    {
        { LedKeyboard::Key::arrow_top },
        { LedKeyboard::Key::arrow_top, LedKeyboard::Key::arrow_right },
        { LedKeyboard::Key::arrow_right, LedKeyboard::Key::arrow_bottom },
        { LedKeyboard::Key::arrow_bottom },
        { LedKeyboard::Key::arrow_bottom, LedKeyboard::Key::arrow_left },
        { LedKeyboard::Key::arrow_left, LedKeyboard::Key::arrow_top },
    };

    std::vector<LedKeyboard::KeyValue> keys;
    for (const auto &key : directions[n])
    {
        keys.push_back(LedKeyboard::KeyValue
        {
            .key = key,
            .color = foreground_color,
        });
    }

    keyboard.setKeys(keys);
}

static void binary_number(LedKeyboard &keyboard, int n)
{
    std::array bit_keys =
    { 
        LedKeyboard::Key::page_down, LedKeyboard::Key::end, LedKeyboard::Key::del,
        LedKeyboard::Key::page_up, LedKeyboard::Key::home, LedKeyboard::Key::insert,
    };

    std::vector<LedKeyboard::KeyValue> keys;
    for (int i = 0; i < bit_keys.size(); i++)
    {
        int bit = (n >> i) & 1;
        if (!bit)
            continue;
        
        keys.push_back(LedKeyboard::KeyValue
        {
            .key = bit_keys[i],
            .color = foreground_color,
        });
    }

    keyboard.setKeys(keys);
}

static char getch() 
{
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

static void interactive_mode(LedKeyboard keyboard)
{
    std::unordered_set<std::string> keys;
    for (;;)
    {
        std::string key_str;
        int state = 0;
        int number = 0;
        for (;;)
        {
            char c = getch();
            key_str = std::string({c});

            switch(state)
            {
                case 0:
                    if (c == '\e')
                        state = 1;
                    break;

                case 1: // [
                    state = 2;
                    break;

                case 2: // X
                    number = (c - '0') * 10;
                    state = 3;
                    break;

                case 3:// X
                    number += c - '0';
                    state = 4;
                    break;

                case 4: // ~
                    state = 0;
                    if (number >= 10 && number <= 15)
                        key_str = "f" + std::to_string((number - 10) + 1);
                    if (number >= 17 && number <= 21)
                        key_str = "f" + std::to_string((number - 17) + 6);
                    if (number >= 23 && number <= 26)
                        key_str = "f" + std::to_string((number - 23) + 11);
                    if (number >= 28 && number <= 29)
                        key_str = "f" + std::to_string((number - 28) + 15);
                    break;
            }

            if (state == 0)
                break;
        }

        std::cout << key_str << "\n";

        if (key_str == "`")
            break;

        bool is_off = !keys.contains(key_str);
        if (is_off)
            keys.insert(key_str);
        else
            keys.erase(key_str);

        LedKeyboard::Key key;
        utils::parseKey(key_str, key);
        keyboard.setKey(LedKeyboard::KeyValue
        {
            .key = key,
            .color = is_off 
                ? LedKeyboard::Color { 0xFF, 0x00, 0xFF }
                : LedKeyboard::Color { 0x00, 0x00, 0x00 },
        });

        keyboard.commit();
    }

    std::cout << "{ ";
    bool is_first = true;
    for (const auto c : keys)
    {
        if (!is_first)
            std::cout << ", ";
        std::cout << "\"" << c << "\"";
        is_first = false;
    }
    std::cout << " },\n";
}

int main(int argc, const char *argv[])
{
    LedKeyboard keyboard;
    if (!keyboard.open(0, 0, ""))
    {
        std::cout << "Failed to connect to keyboard\n";
        return 1;
    }

#if 0

    keyboard.setAllKeys({ 0, 0, 0 });
    keyboard.commit();
    interactive_mode(keyboard);

#else

    for (;;)
    {
        time_t now = time(NULL);
        struct tm *tm_struct = localtime(&now);

        int digit_1 = tm_struct->tm_hour / 10;
        int digit_2 = tm_struct->tm_hour % 10;

        keyboard.setAllKeys(background_color);
        set_digit(keyboard, hour_digit_1[digit_1], foreground_color);
        set_digit(keyboard, hour_digit_2[digit_2], foreground_color);
        direction_number(keyboard, tm_struct->tm_min / 10);
        morse_code_number(keyboard, tm_struct->tm_min % 10);
        binary_number(keyboard, tm_struct->tm_sec);

        keyboard.commit();
        sleep(1);
    }

#endif

    keyboard.commit();
    keyboard.close();
    return 0;
}

