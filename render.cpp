#include "render.hpp"
#include "image.hpp"
#include <cmath>
#include <limits>
#include "octree.h"

atomic_int PROGRESS(0);
double yDiv = 1.f, xDiv = 1.f;
int SuperSamples = 1, DOFRays = 1;
bool bUseOctree = false, bUseDOF = false;
double ApertureRadius = 2.f, FocalDistance = 200;

BoxF GetSceneBounds(const Array<SceneNode*>& Scene)
{
  double inf = 1000000000.f;
  BoxF Bounds(inf, -inf, -inf, inf, -inf, inf);
  for (SceneNode* N : Scene)
  {
    BoxF B = N->GetBox();
    Bounds.GetLeft() = std::min(Bounds.GetLeft(), B.GetLeft());
    Bounds.GetRight() = std::max(Bounds.GetRight(), B.GetRight());
    Bounds.GetTop() = std::max(Bounds.GetTop(), B.GetTop());
    Bounds.GetBottom() = std::min(Bounds.GetBottom(), B.GetBottom());
    Bounds.GetFront() = std::max(Bounds.GetFront(), B.GetFront());
    Bounds.GetBack() = std::min(Bounds.GetBack(), B.GetBack());
  }
  return Bounds;
}

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
               )
{
  std::cout.precision(2);
  std::cerr << "Stub: a4_render(" << root << ",\n     "
          << filename << ", " << width << ", " << height << ",\n     "
          << eye << ", " << view << ", " << up << ", " << fov << ",\n     "
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
  std::cout << "Building octree..." << std::endl;
  OcTree<SceneNode> Tree(GetSceneBounds(List));
  for (SceneNode* s : List)
  {
    // std::cout << s->m_name << "," << s->GetBox() << std::endl;
    Tree.Insert(s);
  }
  // std::cout << Tree << std::endl;

  // Ray setup
  Vector3D normView = view; normView.normalize();
  Vector3D normUp = -up; normUp.normalize();
  Vector3D normRight = normUp.cross(normView); normRight.normalize();

  double radFOV = fov * (M_PI / 180.f);
  double heightWidthRatio = (double)width / (double)height;
  double halfWidth = bUseDOF ? FocalDistance*tan(radFOV/2) : tan(radFOV/2),
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
      threads[i++] = CreateThread<RenderThread> (&List, &Tree, &img, y, endY, x, endX, pixelWidth, pixelHeight, halfWidth, halfHeight,
                                                 eye, normRight, normUp, normView, ambient, &lights, FocalDistance, ApertureRadius);
    }
  }
  
  // Status bar
  while (PROGRESS/xDiv*width < width*height)
  {
    std::cout << std::fixed << (double)(PROGRESS/xDiv*width*100)/((double)(width*height)) << "\%\xd"; std::cout.flush();
    SleepMicro(50000);
  }

  for (int j=0;j<(int)(xDiv*yDiv);j++) threads[j]->Join();

  std::cout << "Creating image file..." << std::endl;
  img.savePng(filename);
}

void RenderThread::Main()
{
  for (int y=yMin;y<yMax;y++)
  {
    for (int x=xMin;x<xMax;x++)
    {
      if (bUseDOF) SetRandomEye();
      Colour Total;
      for (int d=0;d<DOFRays;d++)
      {
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
            SuperTotal.R() += Cols[i].R();
            SuperTotal.G() += Cols[i].G();
            SuperTotal.B() += Cols[i].B();
          }
          SuperTotal = SuperTotal / numRays;
          Total = Total + SuperTotal;
        }
        else
        {
          // Trace through center
          Total = Total + TracePixelNormalized(x, y, 0.5f, 0.5f);
        }
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
  // Vector3D rightComp = ((x * pixelWidth) + (pixelWidth*xNorm) - halfWidth) * normRight;
  // Vector3D upComp = ((y * pixelHeight) + (pixelHeight*yNorm) - halfHeight) * normUp;
  // Vector3D RayDir = normView + rightComp + upComp;
  // RayDir.normalize();

  // Ray CurRay(eye, RayDir);

  Vector3D rightComp = ((x * pixelWidth) + (pixelWidth*xNorm) - halfWidth) * normRight;
  Vector3D upComp = ((y * pixelHeight) + (pixelHeight*yNorm) - halfHeight) * normUp;
  Vector3D View = bUseDOF ? FocalDist*normView : normView;
  Point3D ViewPlanePoint = eye + View + rightComp + upComp;

  // Point3D RandomEyePoint = eye;
  // if (bUseDOF)
  // {
  //   double R1 = ApertureDistribution(generator), R2 = ApertureDistribution(generator);
  //   RandomEyePoint = eye + R1*xApertureRadius + R2*yApertureRadius;
  // }

  Vector3D RayDir = ViewPlanePoint - RandomEyePoint; RayDir.normalize();
  Ray CurRay(RandomEyePoint, RayDir);
  return TraceRay(CurRay, 1.f, 0);
}

Colour RenderThread::TraceRay(Ray& R, double powerCoef, unsigned int depth)
{
  if (powerCoef <= 0.005 || depth >= 10) return Colour();

  Colour TraceColour(0, 0, 0);
  HitInfo Hit;
  if (!Trace(TraceColour, R, Hit, ambient)) return Colour();

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
      double cost = sqrt(1-sin2t);
      double R1 = (ni*NdotR - nt*cost)/(ni*NdotR + nt*cost); R1 *= R1;
      double R2 = (nt*NdotR - ni*cost)/(nt*NdotR + ni*cost); R2 *= R2;
      double Reflectance = (R1 + R2)/2.f;

      Ray ReflectedRay = R.Reflect(Hit, NdotR);
      Colour reflCol;
      if (Hit.Mat->GetGloss() > 0.f)
      {
        // Glossy
        Ray GlossRay; GlossRay.Start = ReflectedRay.Start;
        double theta = acos(pow((1 - RefrDistribution(generator)), 1.f/(Hit.Mat->GetGloss()+1.f)));
        double phi = 2.f * M_PI * RefrDistribution(generator);
        double x = sin(theta) * cos(phi);
        double y = sin(theta) * sin(phi);
        // std::cout << theta << "," << phi << "," << x << "," << y <<  std::endl;
        Vector3D u = ReflectedRay.Direction.cross(Hit.Normal); //u.normalize();
        Vector3D v = ReflectedRay.Direction.cross(u); //v.normalize();
        GlossRay.Direction = x * u + y * v + ReflectedRay.Direction;
        GlossRay.Direction.normalize();
        reflCol = TraceRay(GlossRay, powerCoef * Reflectance, depth + 1);
      }
      else
      {
        reflCol = TraceRay(ReflectedRay, powerCoef * Reflectance, depth + 1);
      }

      Ray RefractedRay = R.Refract(ni, nt, NdotR, sin2t, Hit);
      double refrCoef = powerCoef * (1 - Reflectance);
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

bool RenderThread::Trace(Colour& OutCol, const Ray& R, HitInfo& Hit, const Colour& ambient)
{
  double closestDist = 10000000.f; Matrix4x4 M;
  bool bHit = false;
  Array<SceneNode*> OctList;
  if (bUseOctree && (*Tree).Trace(OctList, R))
  {
    // std::cout << "List:" <<  OctList.Num() << std::endl;
    for (SceneNode* S : OctList)
    {
      if(S->ColourTrace(R, closestDist, Hit, M)) bHit = true;
    }
  }
  else
  {
    // std::cout << "List:" <<  (*List).Num() << std::endl;
    for (SceneNode* S : *List)
    {
      if(S->ColourTrace(R, closestDist, Hit, M)) bHit = true;
    }
  }

  if (bHit)
  {
    Hit.Normal.normalize();

    // Lighting
    // Ambient
    OutCol = ambient*Hit.Mat->GetDiffuse();
    for (Light* L : *lights)
    {
      // Shadows
      if (!IsLightVisible(L->position, Hit.Location)) continue;

      // Diffuse
      Vector3D LightDir = L->position - Hit.Location; LightDir.normalize();
      double LdotN = LightDir.dot(Hit.Normal);
      if (Hit.Mat->GetDiffuse() != Colour(0, 0, 0))
      {
        if (LdotN < 0) LdotN = 0;
        Colour Diffuse = Hit.Mat->GetDiffuse() * L->colour * LdotN;
        OutCol = OutCol + Diffuse;
      }

      // Specular
      if (Hit.Mat->GetSpecular() != Colour(0, 0, 0))
      {
        Vector3D TempEye = R.Start - Hit.Location; TempEye.normalize();
        Vector3D Reflection = ((2*LdotN)*Hit.Normal) - LightDir; Reflection.normalize();
        double RdotE = Reflection.dot(TempEye);
        if (RdotE > 0) OutCol = OutCol + (L->colour * Hit.Mat->GetSpecular() * pow(RdotE, Hit.Mat->GetShine()));
      }
    }
    return true;
  }
  return false;
}

bool RenderThread::IsLightVisible(const Point3D& LightPos, const Point3D& TestLoc)
{
  Vector3D PtToLight = LightPos - TestLoc;
  double LightDist = PtToLight.length();
  PtToLight.normalize();
  double dist = 1000000.f;
  HitInfo ShadowHit; Matrix4x4 M;
  bool bHit = false;
  Array<SceneNode*> OctList;
  if (bUseOctree && (*Tree).Trace(OctList, Ray(TestLoc, PtToLight)))
  {
    for (SceneNode* S : OctList)
    {
      if (S->DepthTrace(Ray(TestLoc, PtToLight), dist, ShadowHit, M)) bHit = true;
    }
  }
  else
  {
    for (SceneNode* S : *List)
    {
      if (S->DepthTrace(Ray(TestLoc, PtToLight), dist, ShadowHit, M)) bHit = true;
    }
  }
  if (bHit && dist > 0.0005 && dist < LightDist) return false;
  return true;
}

void RenderThread::SetRandomEye()
{
  double R1 = ApertureDistribution(generator), R2 = ApertureDistribution(generator);
  RandomEyePoint = eye + R1*xApertureRadius + R2*yApertureRadius;
}