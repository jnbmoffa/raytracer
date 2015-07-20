#include "render.hpp"
#include "image.hpp"
#include <cmath>
#include <limits>
#include "octree.h"
#include "scenecontainer.h"

atomic_int PROGRESS(0);

// TODO: get rid of all globals
double yDiv = 1.f, xDiv = 1.f;
int SuperSamples = 1;
bool bUseOctree = false, bUseAdaptive = false;

void render(// What to render
               SceneNode* root,
               // Where to output the image
               const std::string& filename,
               // Image size
               int width, int height,
               // Viewing parameters
               Camera* cam,
               // Lighting parameters
               const Colour& ambient,
               const std::list<Light*>& lights
               )
{
  // Nice-to-have output
  std::cout.precision(2);
  std::cerr << "render("
          << filename << ", " << width << ", " << height << ",\n     "
          << *cam << ",\n     "
          << ambient << ",\n     {";

  for (std::list<Light*>::const_iterator I = lights.begin(); I != lights.end(); ++I) {
    if (I != lights.begin()) std::cerr << ", ";
    std::cerr << **I;
  }
  std::cerr << "});" << std::endl;

  // Scene setup
  Array<SceneNode*> List;
  root->FlattenScene(List);
  SceneContainer* Scene;
  if (bUseOctree)
  {
     Scene = new OctreeSceneContainer(&List, &lights);
  }
  else
  {
    Scene = new SceneContainer(&List, &lights);
  }
  Scene->MapPhotons();

  // Camera setup
  cam->view = cam->view; cam->view.normalize();
  cam->up = -cam->up; cam->up.normalize();
  cam->right = cam->up.cross(cam->view); cam->right.normalize();
  cam->CalcApertureRadius();

  // Determine the render plane parameters
  RenderPlaneParams Params;
  double radFOV = cam->fov * (M_PI / 180.f);
  double heightWidthRatio = (double)width / (double)height;

  Params.halfWidth = cam->GetFocalDist()*tan(radFOV/2);
  Params.halfHeight = heightWidthRatio * Params.halfWidth;

  double cameraWidth = Params.halfWidth * 2, cameraHeight = Params.halfHeight * 2;

  Params.pixelWidth = cameraWidth / (width - 1);
  Params.pixelHeight = cameraHeight / (height - 1);

  Image img(width, height, 3);

  // Create rendering threads
  std::cout << "Tracing rays..." << std::endl;
  RenderThread* threads[(int)(xDiv*yDiv)];
  int i = 0;
  double H2 = height/yDiv, W3 = width/xDiv;
  for (double y=0;y<height;y+=H2)
  {
    for (double x=0;x<width;x+=W3)
    {
      int endY = ((y+H2) > height) ? height : y+H2;
      int endX = ((x+W3) > width) ? width : x+W3;
      Params.yMin = y; Params.yMax = endY;
      Params.xMin = x; Params.xMax = endX;

      threads[i++] = CreateThread<RenderThread> (Scene, &img, Params, cam, ambient, &lights);
    }
  }
  
  // Status bar
  while (PROGRESS/xDiv*width < width*height)
  {
    std::cout << std::fixed << (double)(PROGRESS/xDiv*width*100)/((double)(width*height)) << "\%\xd"; std::cout.flush();
    SleepMicro(50000);  // Sleep so minimal CPU is used for this
  }

  // Wait for threads to finish
  for (int j=0;j<(int)(xDiv*yDiv);j++) threads[j]->Join();

  std::cout << "Creating image file (" << filename << ")..." << std::endl;
  img.savePng(filename);

  delete Scene;
}

void RenderThread::Main()
{
  for (int y=Params.yMin;y<Params.yMax;y++)
  {
    for (int x=Params.xMin;x<Params.xMax;x++)
    {
      Colour Total;
      for (int d=0;d<NormCam->GetDOFRays();d++)
      {
        RandomEyePoint = NormCam->GetRandomEye();

        // TODO: Find a better way to do this without global variables
        if (SuperSamples > 1)
        {
          // Equally distributed super-samples (grid)
          Colour SuperTotal;
          double numRays = (double)SuperSamples*SuperSamples;
          Colour Cols[(int)numRays];
          double halfSubWidth = 1.f/((double)SuperSamples*2.f); int index = 0;
          for (double i=halfSubWidth;i<1;i+=halfSubWidth*2)
          {
            for (double j=halfSubWidth;j<1;j+=halfSubWidth*2)
            {
              Cols[index++] = TracePixelNormalized(x, y, i, j);
            }
          }

          for (int i=0;i<(int)numRays;i++)
          {
            SuperTotal  = SuperTotal + Cols[i];
          }
          SuperTotal = SuperTotal / numRays;
          Total = Total + SuperTotal;
        }
        else if (bUseAdaptive)
        {
          Total = Total + AdaptiveSuperSample(x, y, 0.f, 1.f, 0.f, 1.f, 0);
        }
        else
        {
          // No Anti-aliasing - trace center
          Total = Total + TracePixelNormalized(x, y, 0.5f, 0.5f);
        }
      }

      // Average DOF ray colour
      Total = Total / NormCam->GetDOFRays();

      (*img)(x, y, 0) = Total.R();
      (*img)(x, y, 1) = Total.G();
      (*img)(x, y, 2) = Total.B();
    }
    PROGRESS++;
  }
}

Colour RenderThread::TracePixelNormalized(int x, int y, double xNorm, double yNorm)
{
  // Determine the location on the render plane to fire the ray through
  // xNorm == 0 && yNorm == 0 -> bottom left of pixel (x,y)
  // xNorm == 1 && yNorm == 1 -> top right of pixel (x,y)
  Vector3D rightComp = ((x * Params.pixelWidth) + (Params.pixelWidth*xNorm) - Params.halfWidth) * NormCam->right;
  Vector3D upComp = ((y * Params.pixelHeight) + (Params.pixelHeight*yNorm) - Params.halfHeight) * NormCam->up;
  Vector3D View = NormCam->GetFocalDist()*NormCam->view;
  Point3D ViewPlanePoint = NormCam->eye + View + rightComp + upComp;

  Vector3D RayDir = ViewPlanePoint - RandomEyePoint; RayDir.normalize();
  Ray CurRay(RandomEyePoint, RayDir);
  return TraceRay(CurRay, 1.f, 0);
}

Colour RenderThread::TraceRay(Ray& R, double powerCoef, unsigned int depth)
{
  if (powerCoef <= 0.05 || depth >= 9) return Colour();

  Colour TraceColour(0, 0, 0);
  HitInfo Hit;
  if (!Scene->RayTrace(TraceColour, R, Hit, ambient)) return Colour();

  // Refraction enabled?
  if (Hit.Mat->GetRef())
  {
    double NdotR = R.Direction.dot(Hit.Normal);
    double ni, nt;
    if (NdotR > 0)
    {
      // Leaving object
      ni = Hit.Mat->GetRefIndex(); nt = 1.f;
      Hit.Normal = -Hit.Normal;
    }
    else
    {
      // Entering object
      ni = 1.f; nt = Hit.Mat->GetRefIndex();
      NdotR = (-R.Direction).dot(Hit.Normal);
    }
    double sin2t = (ni/nt)*(ni/nt)*(1.f-NdotR*NdotR); // assumes R.Direction and Hit.Normal are normalized

    if (sin2t <= 1.f)
    {
      // Fresnel
      double cost = sqrt(1.f-sin2t);
      double R1 = (ni*NdotR - nt*cost)/(ni*NdotR + nt*cost); R1 *= R1;
      double R2 = (nt*NdotR - ni*cost)/(nt*NdotR + ni*cost); R2 *= R2;
      double Reflectance = (R1 + R2)/2.f;

      // Reflected ray creation
      Ray ReflectedRay = R.Reflect(Hit, NdotR);
      Colour reflCol;
      if (Hit.Mat->GetGloss() > 0.f)
      {
        // Glossy
        Ray GlossRay; GlossRay.Start = ReflectedRay.Start;
        Vector3D u = ReflectedRay.Direction.cross(Hit.Normal);
        Vector3D v = ReflectedRay.Direction.cross(u);

        // TODO: remove magic number 4
        Colour TotalGloss;
        for (int i=0;i<4;i++)
        {
          double theta = acos(pow((1.f - RefrDistribution(generator)), 1.f/(Hit.Mat->GetGloss()+1.f)));
          double phi = 2.f * M_PI * RefrDistribution(generator);
          double x = sin(theta) * cos(phi);
          double y = sin(theta) * sin(phi);

          // Perturb ray
          GlossRay.Direction = x * u + y * v + ReflectedRay.Direction;
          GlossRay.Direction.normalize();
          TotalGloss = TotalGloss + TraceRay(GlossRay, powerCoef * Reflectance, depth + 1);
        }
        reflCol = TotalGloss / 4.f;
      }
      else
      {
        reflCol = TraceRay(ReflectedRay, powerCoef * Reflectance, depth + 1);
      }

      // Refracted ray creation
      Ray RefractedRay = R.Refract(ni, nt, NdotR, sin2t, Hit);
      double refrCoef = powerCoef * (1.f - Reflectance);
      Colour refrCol =  refrCoef * TraceColour + TraceRay(RefractedRay, refrCoef, depth + 1);

      return reflCol + refrCol;
    }
    else
    {
      // Total Internal Reflection
      Ray ReflectedRay = R.Reflect(Hit, (-R.Direction).dot(Hit.Normal));
      return TraceRay(ReflectedRay, powerCoef, depth + 1);
    }
  }
  else
  {
    // Diffuse surface
    return powerCoef * TraceColour;
  }
}

Colour RenderThread::AdaptiveSuperSample(int x, int y, double xNormMin, double xNormMax, double yNormMin, double yNormMax, unsigned int depth)
{
  Colour Cols[4];
  double QuartX = (xNormMax - xNormMin)/4.f;
  double QuartY = (yNormMax - yNormMin)/4.f;
  Cols[0] = TracePixelNormalized(x, y, xNormMin + QuartX, yNormMin + QuartY);
  Cols[1] = TracePixelNormalized(x, y, xNormMin + QuartX, yNormMax - QuartY);
  Cols[2] = TracePixelNormalized(x, y, xNormMax - QuartX, yNormMin + QuartY);
  Cols[3] = TracePixelNormalized(x, y, xNormMax - QuartX, yNormMax - QuartY);
  Colour Total = Cols[0] + Cols[1] + Cols[2] + Cols[3];

  // Not the same colours - recurse
  if (!(Cols[0] == Cols[1] && Cols[0] == Cols[2] && Cols[0] == Cols[3]) && depth < 2)
  {
    double HalfX = QuartX*2, HalfY = QuartY*2;
    unsigned int newDepth = depth+1;
    Total = Total + AdaptiveSuperSample(x, y, xNormMin, xNormMin + HalfX, yNormMin, yNormMin + HalfY, newDepth) +
                    AdaptiveSuperSample(x, y, xNormMin + HalfX, xNormMax, yNormMin, yNormMin + HalfY, newDepth) +
                    AdaptiveSuperSample(x, y, xNormMin, xNormMin + HalfX, yNormMin + HalfY, yNormMax, newDepth) +
                    AdaptiveSuperSample(x, y, xNormMin + HalfX, xNormMax, yNormMin + HalfY, yNormMax, newDepth);
    Total = Total / 8.f;
  }
  // Same colours or too deep
  else
  {
    Total = Total / 4.f;
  }

  return Total;
}