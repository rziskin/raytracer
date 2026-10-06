#pragma once

#include "vector.h"

class Intersection {
public:
    Intersection(const Vector& position, const Vector& normal, double distance);

    const Vector& GetPosition() const;
    const Vector& GetNormal() const;
    double GetDistance() const;

private:
    Vector position_;
    Vector normal_;
    double distance_;
};
