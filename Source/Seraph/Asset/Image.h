//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-03 19:03:14
//

#pragma once

#include <Core/Types.h>

struct ImageData
{
    int Width;
    int Height;
    Array<uint8> Pixels;
};

class Image
{
public:
    static void WriteImageData(const ImageData& data, const String& path);
    static void WriteImageRGB(const float* data, int width, int height, const String& path);

    static void ShouldFlipImage(bool flip);
    static ImageData LoadImageData(const String& path);
    static ImageData LoadOnlyRGB(const String& path);
};
