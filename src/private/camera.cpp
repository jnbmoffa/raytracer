#include "camera.h"

// Factory to assist with creating cameras
std::unique_ptr<Camera> CreateCamera( const std::shared_ptr<LuaCamera>& luaCam,
                                      const int width,
                                      const int height )
{
    if ( luaCam->ApertureRadius != 0 )
    {
        return std::unique_ptr<Camera>( new LenseCamera( luaCam, width, height ) );
    }
    else
    {
        return std::unique_ptr<Camera>( new PointCamera( luaCam, width, height ) );
    }
}

std::ostream& operator<<( std::ostream& out, const Camera& c )
{
    auto& cam = c.m_luaCamera;
    return out  << "Camera[" << cam.eye << ", " << cam.view << ", " << cam.up << ", " << cam.fov
           << ", " << cam.ApertureRadius << ", " << cam.FocalDistance << "]";
}