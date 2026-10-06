#pragma once

#include "material.h"
#include "geometry.h"
#include <cstddef>

struct Object {
    const Material* material = nullptr;
    Triangle polygon;
    Vector normals[3];
    bool has_normals[3] = {false, false, false};

    const Vector* GetNormal(size_t index) const {
        if (has_normals[index]) {
            return &normals[index];
        }
        return nullptr;
    }
};

struct SphereObject {
    const Material* material = nullptr;
    Sphere sphere;
};
