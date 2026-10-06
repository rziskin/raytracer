#include "triangle.h"
#include "stdexcept"
Triangle::Triangle(const Vector& a, const Vector& b, const Vector& c) : data_{a, b, c} {
}
const Vector& Triangle::operator[](size_t ind) const {
    if (ind > 2) {
        throw std::out_of_range("Triangle operator[]");
    }
    return data_[ind];
}
double Triangle::Area() const {
    return Length(CrossProduct(data_[0] - data_[1], data_[2] - data_[1])) / 2;
}
