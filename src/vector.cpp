#include "vector.h"
#include <cmath>
#include <stdexcept>
Vector::Vector(double x, double y, double z) : data_{x, y, z} {
}
Vector::Vector() : data_{0, 0, 0} {
}
double Vector::operator[](size_t ind) const {
    if (ind > 2) {
        throw std::out_of_range("Vector operator[]");
    }
    return data_[ind];
}
double& Vector::operator[](size_t ind) {
    if (ind > 2) {
        throw std::out_of_range("Vector operator[]");
    }
    return data_[ind];
}
void Vector::Normalize() {
    double len = Length(*this);
    if (!std::isfinite(len) || len == 0) {
        throw std::invalid_argument("Cannot normalize zero or non-finite vector");
    }
    for (int i = 0; i < 3; i++) {
        data_[i] /= len;
    }
}
double Length(Vector const& a) {
    return sqrt(DotProduct(a, a));
}
double DotProduct(const Vector& a, const Vector& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
Vector CrossProduct(const Vector& a, const Vector& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}
Vector Vector::operator+(const Vector& b) const {
    return {data_[0] + b[0], data_[1] + b[1], data_[2] + b[2]};
}
Vector Vector::operator*(double b) const {
    return {data_[0] * b, data_[1] * b, data_[2] * b};
}
Vector Vector::operator-(const Vector& b) const {
    return {data_[0] - b[0], data_[1] - b[1], data_[2] - b[2]};
}
Vector operator*(double a, const Vector b) {
    return {b[0] * a, b[1] * a, b[2] * a};
}
Vector& Vector::operator+=(const Vector& b) {
    for (int i = 0; i < 3; i++) {
        data_[i] += b[i];
    }
    return *this;
}
Vector& Vector::operator-=(const Vector& b) {
    for (int i = 0; i < 3; i++) {
        data_[i] -= b[i];
    }
    return *this;
}
Vector& Vector::operator*=(double b) {
    for (int i = 0; i < 3; i++) {
        data_[i] *= b;
    }
    return *this;
}
