#include "render.hpp"
#include "image.hpp"
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include "octree.h"
#include "scenecontainer.h"
#include "PixelQueue.h"

atomic_int PROGRESS(0);

// TODO: get rid of all globals
size_t numThreads = 1, SuperSamples = 1; // AA
bool bUseOctree = false, bUseAdaptive = false;

void render( // What to render
    std::unique_ptr<SceneNode>&& root,
    // Where to output the image
    const std::string& filename,
    // Image size
    int width, int height,
    // Viewing parameters
    std::shared_ptr<LuaCamera> luaCam,
    // Lighting parameters
    const Colour& ambient,
    const std::list<std::unique_ptr<Light>>& lights,
    int MappedPhotons,
    // Time stepping
    double TimeDuration, int TimeSteps
)
{
    // Scene setup
    std::vector<std::unique_ptr<SceneNode>> List;
    root->FlattenScene(List);

    std::unique_ptr<SceneContainer> Scene;
    if (bUseOctree)
    {
        Scene = std::make_unique<OctreeSceneContainer>(&List, &lights, MappedPhotons);
    }
    else
    {
        Scene = std::make_unique<SceneContainer>(&List, &lights, MappedPhotons);
    }

    std::unique_ptr<Camera> cam = CreateCamera(luaCam, width, height);

    // Create img object to store the render
    std::unique_ptr<Image> img = std::make_unique<Image>(width, height, 3);

    // Nice-to-have output
    std::cout.precision(2);
    std::cerr << "render("
              << filename << ", " << width << ", " << height << ",\n     "
              << *cam << ",\n     "
              << ambient << ",\n     {";
    for (auto Iter = lights.cbegin(); Iter != lights.cend(); ++Iter)
    {
        if (Iter != lights.cbegin())
        {
            std::cerr << ", ";
        }
        std::cerr << **Iter;
    }
    std::cerr << "});" << std::endl;

    // Setup pixel queue for render threads
    std::unique_ptr<PixelQueue> pixelQueue = std::make_unique<PixelQueue>(width - 1, height - 1);
    // Setup shared render data
    RenderData renderData;
    renderData.m_scene = Scene.get();
    renderData.m_outImage = img.get();
    renderData.m_pixelQueue = pixelQueue.get();
    renderData.m_normCam = cam.get();
    renderData.m_ambient = &ambient;
    renderData.m_timeDuration = TimeDuration;
    renderData.m_timeSteps = TimeSteps;

    // Create rendering threads
    std::cout << "Tracing rays..." << std::endl;
    std::unique_ptr<RenderThread> threads[numThreads];

    for (size_t threadNum = 0; threadNum < numThreads; ++threadNum)
    {
        if (bUseAdaptive)
        {
            threads[threadNum] = CreateThread<AdaptiveSampleThread>(renderData);
        }
        else if (SuperSamples > 1)
        {
            threads[threadNum] = CreateThread<SuperSampleThread>(renderData, SuperSamples);
        }
        else
        {
            threads[threadNum] = CreateThread<RenderThread>(renderData);
        }
    }

    // Status bar
    while (PROGRESS < width * height)
    {
        std::cout << std::fixed << (PROGRESS / static_cast<float>(width * height)) * 100.0 << "\%\xd";
        std::cout.flush();
        SleepMicro(50000);   // Sleep so minimal CPU is used for this
    }

    // Wait for threads to finish
    for (size_t j = 0; j < numThreads; j++)
    {
        threads[j]->Join();
    }

    std::cout << "Creating image file (" << filename << ")..." << std::endl;
    img->savePng(filename);
}

void RenderThread::Main()
{
    for (Pixel pixel = m_renderData.m_pixelQueue->GetNextPixel(); pixel != Pixel::NullPixel; pixel = m_renderData.m_pixelQueue->GetNextPixel())
    {
        const Pixel::StorageType x = pixel.GetX();
        const Pixel::StorageType y = pixel.GetY();
        // std::cout << "\n" << "Rendering pixel (" << x << "," << y << ")" << std::endl;

        Colour Total;
        for (int t = 0; t < m_renderData.m_timeSteps; t++)
        {
            const double Time  = m_renderData.m_timeDuration * ((double)t / (double)m_renderData.m_timeSteps);
            Colour DOFTotal;
            for (int d = 0; d < m_renderData.m_normCam->GetDOFRays(); d++)
            {
                DOFTotal += TracePixelAntiAliased(x, y, Time);
            }
            // Average DOF ray colour
            Total += DOFTotal / m_renderData.m_normCam->GetDOFRays();
        }

        Total /= m_renderData.m_timeSteps;

        (*(m_renderData.m_outImage))(x, y, 0) = Total.R();
        (*(m_renderData.m_outImage))(x, y, 1) = Total.G();
        (*(m_renderData.m_outImage))(x, y, 2) = Total.B();
        ++PROGRESS;
    }
}

inline Colour RenderThread::TracePixelAntiAliased(int x, int y, const double& Time)
{
    Ray ray = m_renderData.m_normCam->GetRayThroughPixel(x, y, 0.5f, 0.5f);
    return TraceRay(ray, 1.0, 0, Time);
}

Colour RenderThread::TraceRay(Ray& ray, double powerCoef, unsigned int depth, const double& Time)
{
    if (powerCoef <= 0.05 || depth >= 9)
    {
        return Colour::Black;
    }

    Colour TraceColour(0, 0, 0);
    HitInfo Hit;
    if (!m_renderData.m_scene->TimeRayTrace(TraceColour, ray, Hit, *(m_renderData.m_ambient), Time))
    {
        return Colour::Black;
    }

    const Vector3D rayDir = ray.GetDirection();

    // Refraction enabled?
    if (Hit.Mat->GetRef())
    {
        const unsigned int nextDepth = depth + 1;

        double NdotR = rayDir.dot(Hit.Normal);
        double ni, nt;
        if (NdotR > 0)
        {
            // Leaving object
            ni = Hit.Mat->GetRefIndex();
            nt = 1.0;
            Hit.Normal = -Hit.Normal;
        }
        else
        {
            // Entering object
            ni = 1.0;
            nt = Hit.Mat->GetRefIndex();
            NdotR = (-rayDir).dot(Hit.Normal);
        }
        const double nint = ni / nt;
        const double sin2t = nint * nint * (1.0 - NdotR * NdotR);   // assumes rayDir and Hit.Normal are normalized

        if (sin2t <= 1.0)
        {
            // Fresnel
            const double cost = sqrt(1.0 - sin2t);
            double R1 = (ni * NdotR - nt * cost) / (ni * NdotR + nt * cost);
            R1 *= R1;
            double R2 = (nt * NdotR - ni * cost) / (nt * NdotR + ni * cost);
            R2 *= R2;
            const double Reflectance = (R1 + R2) * 0.5;

            // Reflected ray creation
            Ray ReflectedRay = ray.Reflect(Hit, NdotR);
            Colour reflectionColour;
            if (Hit.Mat->GetGloss() > 0.0)
            {
                // Glossy
                const Vector3D reflRayDir = ReflectedRay.GetDirection();
                const Vector3D u = cross(reflRayDir, Hit.Normal);
                const Vector3D v = cross(reflRayDir, u);

                // TODO: remove magic number 8
                Colour TotalGloss;
                for (int i = 0; i < 8; i++)
                {
                    const double theta = acos(pow((1.0 - RefrDistribution(generator)), 1.0 / (Hit.Mat->GetGloss() + 1.0)));
                    const double phi = 2.0 * M_PI * RefrDistribution(generator);
                    const double x = sin(theta) * cos(phi);
                    const double y = sin(theta) * sin(phi);

                    // Perturb ray
                    Ray GlossRay(ReflectedRay.GetOrigin(), x * u + y * v + reflRayDir);
                    TotalGloss += TraceRay(GlossRay, powerCoef * Reflectance, nextDepth, Time);
                }
                reflectionColour = TotalGloss * 0.125;
            }
            else
            {
                // Total internal reflection
                reflectionColour = TraceRay(ReflectedRay, powerCoef * Reflectance, nextDepth, Time);
            }

            // Refracted ray creation
            Ray RefractedRay = ray.Refract(ni, nt, NdotR, sin2t, Hit);
            const double refrCoef = powerCoef * (1.0 - Reflectance);
            const Colour refractionColour = (refrCoef * TraceColour) + TraceRay(RefractedRay, refrCoef, nextDepth, Time);

            return reflectionColour + refractionColour;
        }
        else
        {
            // Total Internal Reflection
            Ray ReflectedRay = ray.Reflect(Hit, (-rayDir).dot(Hit.Normal));
            return TraceRay(ReflectedRay, powerCoef, nextDepth, Time);
        }
    }
    else
    {
        // Diffuse surface
        return powerCoef * TraceColour;
    }
}

Colour SuperSampleThread::TracePixelAntiAliased(int x, int y, const double& Time)
{
    // Equally distributed super-samples (grid)
    const double halfSubWidth = 0.5 / SuperSamples;
    const size_t numRays = SuperSamples * SuperSamples;
    Colour Cols[numRays];
    uint32_t index = 0;
    for (double i = halfSubWidth; i < 1; i += halfSubWidth * 2)
    {
        for (double j = halfSubWidth; j < 1; j += halfSubWidth * 2)
        {
            Ray ray = m_renderData.m_normCam->GetRayThroughPixel(x, y, i, j);
            Cols[index++] = TraceRay(ray, 1.0, 0, Time);
        }
    }

    Colour SuperTotal;
    for (size_t i = 0; i < numRays; i++)
    {
        SuperTotal += Cols[i];
    }
    return SuperTotal / static_cast<double>(numRays);
}

inline Colour AdaptiveSampleThread::TracePixelAntiAliased(int x, int y, const double& Time)
{
    return AdaptiveSuperSample(x, y, 0.0, 1.0, 0.0, 1.0, 0, Time);
}

Colour AdaptiveSampleThread::AdaptiveSuperSample(const int x, const int y, const double xNormMin, const double xNormMax, const double yNormMin, const double yNormMax, const unsigned int depth, const double& Time)
{
    Colour Cols[4];
    const double QuartX = (xNormMax - xNormMin) * 0.25;
    const double QuartY = (yNormMax - yNormMin) * 0.25;
    Ray R0 = m_renderData.m_normCam->GetRayThroughPixel(x, y, xNormMin + QuartX, yNormMin + QuartY);
    Ray R1 = m_renderData.m_normCam->GetRayThroughPixel(x, y, xNormMin + QuartX, yNormMax - QuartY);
    Ray R2 = m_renderData.m_normCam->GetRayThroughPixel(x, y, xNormMax - QuartX, yNormMin + QuartY);
    Ray R3 = m_renderData.m_normCam->GetRayThroughPixel(x, y, xNormMax - QuartX, yNormMax - QuartY);
    Cols[0] = TraceRay(R0, 1.0, 0, Time);
    Cols[1] = TraceRay(R1, 1.0, 0, Time);
    Cols[2] = TraceRay(R2, 1.0, 0, Time);
    Cols[3] = TraceRay(R3, 1.0, 0, Time);
    Colour Total = Cols[0] + Cols[1] + Cols[2] + Cols[3];

    // Not the same colours - recurse
    if (!(Cols[0] == Cols[1] && Cols[0] == Cols[2] && Cols[0] == Cols[3]) && depth < 2)
    {
        const double HalfX = QuartX * 2;
        const double HalfY = QuartY * 2;
        const unsigned int newDepth = depth + 1;

        Total += AdaptiveSuperSample(x, y, xNormMin, xNormMin + HalfX, yNormMin, yNormMin + HalfY, newDepth, Time) +
                 AdaptiveSuperSample(x, y, xNormMin + HalfX, xNormMax, yNormMin, yNormMin + HalfY, newDepth, Time) +
                 AdaptiveSuperSample(x, y, xNormMin, xNormMin + HalfX, yNormMin + HalfY, yNormMax, newDepth, Time) +
                 AdaptiveSuperSample(x, y, xNormMin + HalfX, xNormMax, yNormMin + HalfY, yNormMax, newDepth, Time);

        Total *= 0.125;
    }
    // Same colours or too deep
    else
    {
        Total *= 0.25;
    }

    return Total;
}