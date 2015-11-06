#include "photon.h"

std::ostream& operator<<(std::ostream& out, const Photon& p)
{
    return out << "Photon[" << p.Position << "," << p.Power << "," << p.IncidentDir;
}