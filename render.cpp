#include "render.hpp"
#include "image.hpp"
#include <cmath>
#include <limits>
#include "octree.h"
#include "scenecontainer.h"

atomic_int PROGRESS(0);
double yDiv = 1.f, xDiv = 1.f;
int SuperSamples = 1, DOFRays = 1;
bool bUseOctree = false, bUseDOF = false, bUseAdaptive = false;

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

  // Octree
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

  // Ray setup
  cam->view = cam->view; cam->view.normalize();
  cam->up = -cam->up; cam->up.normalize();
  cam->right = cam->up.cross(cam->view); cam->right.normalize();
  cam->CalcApertureRadius();

  double radFOV = cam->fov * (M_PI / 180.f);
  double heightWidthRatio = (double)width / (double)height;
  double halfWidth = bUseDOF ? cam->FocalDistance*tan(radFOV/2) : tan(radFOV/2),
         halfHeight = heightWidthRatio * halfWidth,
         cameraWidth = halfWidth * 2,
         cameraHeight = halfHeight * 2,
         pixelWidth = cameraWidth / (width - 1),
         pixelHeight = cameraHeight / (height - 1);

  Image img(width, height, 3);

  // Create rendering threads
  std::cout << "Casting rays..." << std::endl;
  RenderThread* threads[(int)(xDiv*yDiv)];
  int i = 0;
  double H2 = height/yDiv, W3 = width/xDiv;
  for (double y=0;y<height;y+=H2)
  {
    for (double x=0;x<width;x+=W3)
    {
      int endY = ((y+H2) > height) ? height : y+H2;
      int endX = ((x+W3) > width) ? width : x+W3;
      threads[i++] = CreateThread<RenderThread> (Scene, &img, y, endY, x, endX, pixelWidth, pixelHeight, halfWidth, halfHeight,
                                                 cam, ambient, &lights);
    }
  }
  
  // Status bar
  while (PROGRESS/xDiv*width < width*height)
  {
    std::cout << std::fixed << (double)(PROGRESS/xDiv*width*100)/((double)(width*height)) << "\%\xd"; std::cout.flush();
    SleepMicro(50000);
  }

  for (int j=0;j<(int)(xDiv*yDiv);j++) threads[j]->Join();

  std::cout << "Creating image file (" << filename << ")..." << std::endl;
  img.savePng(filename);

  delete Scene;
}

void RenderThread::Main()
{
  for (int y=yMin;y<yMax;y++)
  {
    for (int x=xMin;x<xMax;x++)
    {
      Colour Total;
      for (int d=0;d<DOFRays;d++)
      {
        if (bUseDOF) RandomEyePoint = NormCam->GetRandomEye();
        // std::cout << RandomEyePoint << std::endl;
        if (SuperSamples > 1)
        {
          Colour SuperTotal;
          // Equally distributed super-samples (grid)
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
        // std::cout << Total << std::endl;
      }

      // Average DOF ray colour
      Total = Total / DOFRays;

      (*img)(x, y, 0) = Total.R();
      (*img)(x, y, 1) = Total.G();
      (*img)(x, y, 2) = Total.B();
    }
    PROGRESS++;
  }
}

Colour RenderThread::TracePixelNormalized(int x, int y, double xNorm, double yNorm)
{
  Vector3D rightComp = ((x * pixelWidth) + (pixelWidth*xNorm) - halfWidth) * NormCam->right;
  Vector3D upComp = ((y * pixelHeight) + (pixelHeight*yNorm) - halfHeight) * NormCam->up;
  Vector3D View = bUseDOF ? NormCam->FocalDistance*NormCam->view : NormCam->view;
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

      Ray ReflectedRay = R.Reflect(Hit, NdotR);
      Colour reflCol;
      if (Hit.Mat->GetGloss() > 0.f)
      {
        // Glossy
        Ray GlossRay; GlossRay.Start = ReflectedRay.Start;
        Vector3D u = ReflectedRay.Direction.cross(Hit.Normal);
        Vector3D v = ReflectedRay.Direction.cross(u);

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