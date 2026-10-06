#include "geometry.h"
#include <optional>
#include <cmath>

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    Vector oc = ray.GetOrigin() - sphere.GetCenter();

    double a = DotProduct(ray.GetDirection(), ray.GetDirection());
    double b = 2.0 * DotProduct(oc, ray.GetDirection());
    double c = DotProduct(oc, oc) - sphere.GetRadius() * sphere.GetRadius();

    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return std::nullopt;
    }

    double sqrt_d = sqrt(discriminant);
    double t1 = (-b - sqrt_d) / (2.0 * a);
    double t2 = (-b + sqrt_d) / (2.0 * a);

    double t = t1;
    if (t < 0.0) {
        t = t2;
    }
    if (t < 0.0) {
        return std::nullopt;
    }

    Vector position = ray.GetOrigin() + ray.GetDirection() * t;
    Vector normal = position - sphere.GetCenter();
    normal.Normalize();

    if (DotProduct(normal, ray.GetDirection()) > 0.0) {
        normal *= -1.0;
    }

    return Intersection{position, normal, Length(position - ray.GetOrigin())};
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    Vector edge1 = triangle[1] - triangle[0];
    Vector edge2 = triangle[2] - triangle[0];

    Vector h = CrossProduct(ray.GetDirection(), edge2);
    double a = DotProduct(edge1, h);

    if (std::fabs(a) < 1e-9) {
        return std::nullopt;
    }

    double f = 1.0 / a;
    Vector s = ray.GetOrigin() - triangle[0];
    double u = f * DotProduct(s, h);

    if (u < 0.0 || u > 1.0) {
        return std::nullopt;
    }

    Vector q = CrossProduct(s, edge1);
    double v = f * DotProduct(ray.GetDirection(), q);

    if (v < 0.0 || u + v > 1.0) {
        return std::nullopt;
    }

    double t = f * DotProduct(edge2, q);

    if (t >= 0.0) {
        Vector position = ray.GetOrigin() + ray.GetDirection() * t;
        Vector normal = CrossProduct(edge1, edge2);
        normal.Normalize();

        if (DotProduct(normal, ray.GetDirection()) > 0.0) {
            normal *= -1.0;
        }

        return Intersection{position, normal, Length(position - ray.GetOrigin())};
    }

    return std::nullopt;
}

Vector Reflect(const Vector& ray, const Vector& normal) {
    return ray - normal * (2.0 * DotProduct(ray, normal));
}

std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    double dot_ni = DotProduct(normal, ray);
    Vector n = normal;

    if (dot_ni > 0.0) {
        n = normal * -1.0;
        dot_ni = DotProduct(n, ray);
    }

    double k = 1.0 - eta * eta * (1.0 - dot_ni * dot_ni);
    if (k < 0.0) {
        return std::nullopt;
    }

    return ray * eta - n * (eta * dot_ni + std::sqrt(k));
}

Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    Vector v0 = triangle[0];
    Vector v1 = triangle[1];
    Vector v2 = triangle[2];

    Vector n = CrossProduct(v1 - v0, v2 - v0);
    double area_squared = DotProduct(n, n);

    Vector p0 = v0 - point;
    Vector p1 = v1 - point;
    Vector p2 = v2 - point;

    double w0 = DotProduct(CrossProduct(p1, p2), n) / area_squared;
    double w1 = DotProduct(CrossProduct(p2, p0), n) / area_squared;
    double w2 = DotProduct(CrossProduct(p0, p1), n) / area_squared;

    return Vector(w0, w1, w2);
}
