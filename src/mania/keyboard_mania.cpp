#include "keyboard.hpp"
#include "osu_parser.hpp"
#include "playback.hpp"
#include "render.hpp"

#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <set>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <raylib.h>

using namespace Mania;

static void play_song(LedKeyboard& keyboard)
{
    // FIXME: This is a REALLY bad hack for disabling the normal capslock
    //        behaviour. Obviously, this only works on X11.
    system("setxkbmap -option caps:none");

    InitWindow(100, 100, "Keyboard Mania");
    SetTargetFPS(60);

    auto osu = parse_osu_file("test.osu");
    auto state = initialize_game_state(osu);
    float frame_timer = 0;

    while (!WindowShouldClose()) {
        state.time += GetFrameTime() / 2.0;

        frame_timer += GetFrameTime();
        if (frame_timer > 0.2) {
            render_frame(keyboard, state);
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
