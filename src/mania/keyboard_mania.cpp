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
    InitAudioDevice();
    SetTargetFPS(30);

    auto osu = parse_osu_file("test2.osu");
    auto state = initialize_game_state(osu);
    float frame_timer = 0;

    auto music = LoadSound("audio.mp3");
    SetSoundVolume(music, 0.1);
    PlaySound(music);

    while (!WindowShouldClose()) {
        state.time += static_cast<float>(GetFrameTime() / 2.0);

        if (IsSoundPlaying(music)) {
            frame_timer += GetFrameTime();
            if (frame_timer > 0.2) {
                render_frame(keyboard, state);
                register_misses(state);
                frame_timer = 0;
            }
        }

        BeginDrawing();
            handle_input(state);
        EndDrawing();
    }

    UnloadSound(music);
    CloseAudioDevice();
    CloseWindow();
}

int main()
{
    LedKeyboard keyboard;
    if (!keyboard.open()) {
        std::cerr << "Error: Unable to connect to keyboard\n";
        return 1;
    }

    play_song(keyboard);
    keyboard.close();
    return 0;
}
