//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-03 19:04:19
//

#include "Image.h"

#include <Core/Context.h>
#include <stb_image_write.h>

void Image::WriteImageData(const ImageData& data, const StringView& path)
{
    stbi_write_png(path.data(), data.Width, data.Height, 4, data.Pixels.data(), data.Width * 4);
    SERAPH_INFO("Wrote image file to %s", path.data());
}
