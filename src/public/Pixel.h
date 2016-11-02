#pragma once

#include <cstdint>

// Basic definition for a pixel in an image
class Pixel
{
public:
    using StorageType = int32_t;

    Pixel(StorageType x, StorageType y): x(x), y(y) {}

    StorageType GetX() const { return x; }
    StorageType GetY() const { return y; }

    static const Pixel NullPixel;

private:
    StorageType x;
    StorageType y;
};

bool operator!=(const Pixel& a, const Pixel& b);