#include "ray.h"

Ray::Ray(const Vector& origin, const Vector& direction) : origin_(origin), dir_(direction) {
    dir_.Normalize();
}

const Vector& Ray::GetOrigin() const {
    return origin_;
}

const Vector& Ray::GetDirection() const {
    return dir_;
}
