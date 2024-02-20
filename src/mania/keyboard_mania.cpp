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

static void handle_input(GameState &state)
{
    if (IsKeyPressed(KEY_GRAVE)) { on_key_pressed(state, 0); }
    if (IsKeyPressed(KEY_TAB)) { on_key_pressed(state, 1); }
    if (IsKeyPressed(KEY_CAPS_LOCK)) { on_key_pressed(state, 2); }
    if (IsKeyPressed(KEY_LEFT_SHIFT)) { on_key_pressed(state, 3); }

    if (IsKeyReleased(KEY_GRAVE)) { on_key_released(state, 0); }
    if (IsKeyReleased(KEY_TAB)) { on_key_released(state, 1); }
    if (IsKeyReleased(KEY_CAPS_LOCK)) { on_key_released(state, 2); }
    if (IsKeyReleased(KEY_LEFT_SHIFT)) { on_key_released(state, 3); }
}

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
            register_misses(state);
            frame_timer = 0;
        }

        BeginDrawing();
        handle_input(state);
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
