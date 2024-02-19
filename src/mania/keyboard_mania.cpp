#include "keyboard.hpp"
#include "osu_parser.hpp"
#include "playback.hpp"
#include "render.hpp"

#include <algorithm>
#include <iostream>
#include <set>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <math.h>
#include <time.h>

#include <raylib.h>

using namespace Mania;

static void play_song(LedKeyboard& keyboard)
{
    InitWindow(100, 100, "Keyboard Mania");
    SetTargetFPS(60);

    auto osu = parse_osu_file("test.osu");
    auto notes = build_note_sequence(osu);

    LedKeyboard::Color hit_indicator = { .green = 0xFF };
    float time = 0;
    float health = 0.7;

    int count = 0;
    while (!WindowShouldClose()) {
        if (count > 10) {
            render_frame(keyboard, notes, hit_indicator, health, time);
            apply_note_step(notes, 1);
            time += 0.1f;
            count = 0;
        }

        usleep(10 * 1000);
        count += 1;
    }

    CloseWindow();
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
