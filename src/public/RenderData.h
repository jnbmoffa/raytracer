#pragma once

class SceneContainer;
class Image;
class PixelQueue;
class Camera;
class Colour;

/** Data needed for a render thread to render a scene */
struct RenderData
{
    const SceneContainer* m_scene;

    // Not protected by mutex because threads work on separate pixels
    Image* m_outImage;

    PixelQueue* m_pixelQueue;

    const Camera* m_normCam;
    const Colour* m_ambient;

    double m_timeDuration;
    int m_timeSteps;
};