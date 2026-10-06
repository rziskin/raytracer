#pragma once

#include "vector.h"

#include <cstddef>

class Triangle {
public:
    Triangle(const Vector& a, const Vector& b, const Vector& c);

    const Vector& operator[](size_t ind) const;
    double Area() const;

private:
    std::array<Vector, 3> data_;
};
