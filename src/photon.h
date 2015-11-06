#pragma once
#include "kdobject.h"
#include "algebra.hpp"

// The actual stored data in the map
class Photon : public KDObject
{
public:
    Photon() {}
    Photon(Point3D Position, Colour Power, Vector3D IncidentDir) : Position(Position), Power(Power), IncidentDir(IncidentDir) {}

    Point3D Position;       // Hit location
    Colour Power;           // Contributed light
    Vector3D IncidentDir;

    virtual double operator[](unsigned int Index) const override
    {
        return Position[Index];
    }
};

std::ostream& operator<<(std::ostream& out, const Photon& p);