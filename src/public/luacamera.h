#pragma once

#include "algebra.hpp"
#include <iosfwd>

// Used by LUA to gather the information for a camera object
struct LuaCamera {
    Point3D eye;
    Vector3D view, up, right;
    double fov, ApertureRadius, FocalDistance;
    int DOFRays;

    LuaCamera() :   eye(0.f,0.f,0.f),
                    view(0.f,0.f,-1.f),
                    up(0.f,1.f,0.f),
                    fov(50.f),
                    ApertureRadius(1.4f),
                    FocalDistance(1.f) {}
};