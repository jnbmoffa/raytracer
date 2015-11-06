#pragma once

// The base class of any object to be stored in the KDTree
class KDObject
{
public:
    virtual ~KDObject() {}

    // Need to be able to access the k-th element
    virtual double operator[](unsigned int Index) const = 0;
};