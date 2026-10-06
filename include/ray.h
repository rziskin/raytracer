#pragma once

#include "vector.h"

class Ray {
public:
    Ray(const Vector& origin, const Vector& direction);

    const Vector& GetOrigin() const;
    const Vector& GetDirection() const;

private:
    Vector origin_;
    Vector dir_;
};
