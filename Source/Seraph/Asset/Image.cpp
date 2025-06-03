//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-03 19:04:19
//

#include "Image.h"

#include <Core/Context.h>
#include <stb_image_write.h>
#include <stb_image.h>

void Image::WriteImageData(const ImageData& data, const StringView& path)
{
    stbi_write_png(path.data(), data.Width, data.Height, 4, data.Pixels.data(), data.Width * 4);
    SERAPH_INFO("Wrote image file to %s", path.data());
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
