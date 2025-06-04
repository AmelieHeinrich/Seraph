//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-03 19:04:19
//

#include "Image.h"

#include <Core/Context.h>
#include <stb_image_write.h>
#include <stb_image.h>

uint8 LinearToRGBA8(float linear)
{
    linear = std::clamp(linear, 0.0f, 1.0f);
    float srgb = (linear <= 0.0031308f)
        ? (12.92f * linear)
        : (1.055f * powf(linear, 1.0f / 2.4f) - 0.055f);
    return static_cast<uint8>(std::round(srgb * 255.0f));
}

void ConvertToRGBA8(const float* linearRGB, uint8_t* outRGBA8, int width, int height)
{
    for (int i = 0; i < width * height; ++i) {
        outRGBA8[i * 4 + 0] = LinearToRGBA8(linearRGB[i * 3 + 0]); // R
        outRGBA8[i * 4 + 1] = LinearToRGBA8(linearRGB[i * 3 + 1]); // G
        outRGBA8[i * 4 + 2] = LinearToRGBA8(linearRGB[i * 3 + 2]); // B
        outRGBA8[i * 4 + 3] = 255;
    }
}

void Image::WriteImageData(const ImageData& data, const StringView& path)
{
    stbi_write_png(path.data(), data.Width, data.Height, 4, data.Pixels.data(), data.Width * 4);
    SERAPH_INFO("Wrote image file to %s", path.data());
}

void Image::WriteImageRGB(const float* data, int width, int height, const StringView& path)
{
    uint8* temp = new uint8[width * height * 4];
    ConvertToRGBA8(data, temp, width, height);

    stbi_write_png(path.data(), width, height, 4, temp, width * 4);
    SERAPH_INFO("Wrote image file to %s", path.data());

    delete temp;
}

ImageData Image::LoadImageData(const StringView& path)
{
    ImageData result = {};

    stbi_set_flip_vertically_on_load(true);

    int channels;
    uint8* data = stbi_load(path.data(), &result.Width, &result.Height, &channels, STBI_rgb_alpha);
    result.Pixels.resize(result.Width * result.Height * 4);
    std::copy(data, data + result.Pixels.size(), result.Pixels.begin());
    stbi_image_free(data);

    return result;
}

ImageData Image::LoadOnlyRGB(const StringView& path)
{
    ImageData result = {};

    stbi_set_flip_vertically_on_load(true);

    int channels;
    uint8* data = stbi_load(path.data(), &result.Width, &result.Height, &channels, STBI_rgb);
    result.Pixels.resize(result.Width * result.Height * 3);
    std::copy(data, data + result.Pixels.size(), result.Pixels.begin());
    stbi_image_free(data);

    return result;
}
