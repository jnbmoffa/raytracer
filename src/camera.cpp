#include "camera.h"

std::ostream& operator<<(std::ostream& out, const Camera& c)
{
  return out << "Camera[" << c.eye << ", " << c.view << ", " << c.up << ", " << c.fov <<
  ", " << c.ApertureRadius << ", " << c.FocalDistance << "]";
}