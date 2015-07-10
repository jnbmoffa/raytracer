#ifndef RENDER_HPP
#define RENDER_HPP

#include <string>
#include "algebra.hpp"
#include "scene.hpp"
#include "light.hpp"
#include "Thread.h"
#include <random>

typedef std::list<SceneNode*> NodeList;
extern double xDiv, yDiv;
extern int SuperSamples;

void render(// What to render
               SceneNode* root,
               // Where to output the image
               const std::string& filename,
               // Image size
               int width, int height,
               // Viewing parameters
               const Point3D& eye, const Vector3D& view,
               const Vector3D& up, double fov,
               // Lighting parameters
               const Colour& ambient,
               const std::list<Light*>& lights
               );

class Image;

class RenderThread : public Thread
{
protected:
     NodeList List;
     Image* img;

     int yMin, yMax, xMin, xMax;
     double pixelWidth, pixelHeight, halfWidth, halfHeight;

     const Point3D& eye;
     const Vector3D& normRight;
     const Vector3D& normUp;
     const Vector3D& normView;

     const Colour& ambient;
     const std::list<Light*>* lights;

     std::default_random_engine generator;
     std::uniform_real_distribution<double> distribution;

public:
     RenderThread(NodeList L, Image* img, int yMin, int yMax, int xMin, int xMax, double pixelWidth, double pixelHeight, double halfWidth, double halfHeight,
                  const Point3D& eye, const Vector3D& normRight, const Vector3D& normUp, const Vector3D& normView,
                  const Colour& ambient, const std::list<Light*>* lights) :
               List(L), img(img), yMin(yMin), yMax(yMax), xMin(xMin), xMax(xMax), pixelWidth(pixelWidth), pixelHeight(pixelHeight),
               halfWidth(halfWidth), halfHeight(halfHeight), eye(eye), normRight(normRight), normUp(normUp), normView(normView),
               ambient(ambient), lights(lights), distribution(0.0,1.0) {}

     virtual void Main() override;

     // Do a trace through the pixel at a specific location (center is xNorm=0.5, yNorm=0.5)
     Colour TracePixelNormalized(int x, int y, double xNorm, double yNorm);

     Colour TraceRay(Ray& R, double powerCoef, unsigned int depth);

     bool Trace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient);
     bool IsLightVisible(const Point3D& LightPos, const Point3D& TestLoc);
};

#endif
