#include "keyboard.h"
#include "keyboard_layout.h"
#include <iostream>
#include <array>
#include <optional>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static LedKeyboard::Color average_color(
        uint8_t *data, int image_width, int image_components,
        int sample_x, int sample_y, int sample_with, int sample_height)
{
    uint64_t color_sum[] = { 0, 0, 0 };
    for (int y = sample_y; y < sample_y + sample_height; y++)
    {
        for (int x = sample_x; x < sample_x + sample_with; x++)
        {
            int offset = (y*image_width+x)*image_components;
            color_sum[0] += data[offset + 0];
            color_sum[1] += data[offset + 1];
            color_sum[2] += data[offset + 2];
        }
    }

    int sample_count = sample_with * sample_height;
    return 
    {
        (uint8_t)(color_sum[0] / sample_count),
        (uint8_t)(color_sum[1] / sample_count),
        (uint8_t)(color_sum[2] / sample_count),
    };
}

static std::vector<LedKeyboard::KeyValue> load_image(const char *image_path)
{
    int width, height, components;
    auto *data = stbi_load(image_path, &width, &height, &components, 0);
    if (!data)
    {
        std::cerr << "Error: Unable to load image '" << image_path << "'\n";
        return {};
    }

    std::vector<LedKeyboard::KeyValue> keys;
    auto row_count = (int)Layout::map.size();
    auto column_count = (int)Layout::map[0].size();

    auto pixels_per_led_width = std::max(width / column_count, 1);
    auto pixels_per_led_height = std::max(height / row_count, 1);
    for (int row = 0; row < row_count; row++)
    {
        for (int column = 0; column < column_count; column++)
        {
            auto key = Layout::map[row][column];
            if (!key)
                continue;

            int image_x = column * pixels_per_led_width;
            int image_y = row * pixels_per_led_height;
            auto color = average_color(data, width, components, 
                image_x, image_y, pixels_per_led_width, pixels_per_led_height);

            keys.push_back(LedKeyboard::KeyValue
            {
                .key = *key,
                .color = color,
            });
        }
    }

    stbi_image_free(data);
    return keys;
}

std::vector<LedKeyboard::KeyValue> blend_frames(
    std::vector<LedKeyboard::KeyValue> from, std::vector<LedKeyboard::KeyValue> to, 
    float amount)
{
    std::vector<LedKeyboard::KeyValue> result;
    for (const auto &[key, from_color] : from)
    {
        std::optional<LedKeyboard::Color> to_color = std::nullopt;
        for (const auto &[other_key, color] : to)
        {
            if (key == other_key)
            {
                to_color = color;
                break;
            }
        }

        if (!to_color)
            continue;

        result.push_back(LedKeyboard::KeyValue
        {
            .key = key,
            .color =
            {
                (uint8_t)(from_color.red*(1.0 - amount) + to_color->red*amount),
                (uint8_t)(from_color.green*(1.0 - amount) + to_color->green*amount),
                (uint8_t)(from_color.blue*(1.0 - amount) + to_color->blue*amount),
            },
        });
    }

    return result;
}

int main(int argc, const char *argv[])
{
    if (getuid() != 0)
    {
        std::cerr << "Error: Can be only run as root\n";
        return 1;
    }

    if (argc <= 1)
    {
        std::cerr << "Error: Must provide at least one image file\n";
        return 1;
    }

    LedKeyboard keyboard;
    if (!keyboard.open())
    {
        std::cerr << "Error: Unable to connect to keyboard\n";
        return 1;
    }

    std::vector<std::vector<LedKeyboard::KeyValue>> frames;
    for (int i = 1; i < argc; i++)
        frames.push_back(load_image(argv[i]));

    int frame_time = 100;
    int step_size = 10;

    int frame = 0;
    for (;;)
    {
        for (int i = 0; i < frame_time; i += step_size)
        {
            const auto &curr_frame = frames[frame];
            const auto &next_frame = frames[(frame + 1) % frames.size()];

            float progress =
                (float)i / (float)frame_time;
            keyboard.setKeys(blend_frames(curr_frame, next_frame, progress));
            keyboard.commit();
            usleep(1000 * step_size);
        }
        frame = (frame + 1) % frames.size();
    }

    keyboard.close();
    return 0;
}

