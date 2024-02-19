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
    float health = 0.7;
    float time = 0;
    float frame_timer = 0;

    while (!WindowShouldClose()) {
        frame_timer += GetFrameTime();
        time += GetFrameTime();
        if (frame_timer > 0.2) {
            render_frame(keyboard, notes, hit_indicator, health, time / 2.0);
            frame_timer = 0;
        }

        BeginDrawing();
        EndDrawing();
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
