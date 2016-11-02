#include "PixelQueue.h"

PixelQueue::PixelQueue(DimensionType maxX, DimensionType maxY) :
    m_currentX(0),
    m_currentY(0),
    m_maxX(maxX),
    m_maxY(maxY)
{

}

/**
 *  Get the next pixel in the queue
 *  @return the next pixel or NullPixel if no more exist
 */
Pixel PixelQueue::GetNextPixel()
{
    ScopeLock lock(m_mutex);

    if (m_currentX > m_maxX)
    {
        m_currentX = 0;
        ++m_currentY;
    }
    if (m_currentY > m_maxY)
    {
        return Pixel::NullPixel;
    }

    // return the current pixel and progress to the next
    return Pixel(m_currentX++, m_currentY);
}