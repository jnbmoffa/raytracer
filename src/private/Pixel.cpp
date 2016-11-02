#include "Pixel.h"

const Pixel Pixel::NullPixel = Pixel(-1, -1);

bool operator!=(const Pixel& a, const Pixel& b)
{
    return (a.GetX() != b.GetX()) || (a.GetY() != b.GetY());
}