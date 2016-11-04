#pragma once

#include "Pixel.h"
#include "Thread.h"

/** A manager of pixels for the render threads to pull from to get their next job. */
class PixelQueue
{
public:
    using DimensionType = typename Pixel::StorageType;

    PixelQueue(DimensionType maxX, DimensionType maxY);

    /**
     *  Get the next pixel in the queue
     *  @return the next pixel or NullPixel if no more exist
     */
    Pixel GetNextPixel();

private:
    Mutex m_mutex;

    /** Assumes the pixels start from [0,0] */
    DimensionType m_currentX;
    DimensionType m_currentY;
    DimensionType m_maxX;
    DimensionType m_maxY;
};