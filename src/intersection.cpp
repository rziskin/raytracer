#include "intersection.h"

Intersection::Intersection(const Vector& position, const Vector& normal, double distance)
    : position_(position), normal_(normal), distance_(distance) {
    normal_.Normalize();
}

const Vector& Intersection::GetPosition() const {
    return position_;
}
const Vector& Intersection::GetNormal() const {
    return normal_;
}
double Intersection::GetDistance() const {
    return distance_;
}
