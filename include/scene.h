#pragma once

#include "light.h"
#include "object.h"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class Scene {
public:
    Scene() = default;

    Scene(const Scene& other);
    Scene& operator=(const Scene& other);

    Scene(Scene&& other) noexcept;
    Scene& operator=(Scene&& other) noexcept;
    ~Scene() = default;

    const std::vector<Object>& GetObjects() const;
    const std::vector<SphereObject>& GetSphereObjects() const;
    const std::vector<Light>& GetLights() const;
    const std::unordered_map<std::string, Material>& GetMaterials() const;

    void Load(const std::filesystem::path& path);

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> spheres_;
    std::vector<Light> lights_;
    std::unordered_map<std::string, Material> materials_;
};

Scene ReadScene(const std::filesystem::path& path);
