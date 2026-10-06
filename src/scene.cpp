#include "scene.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

Scene::Scene(const Scene& other) {

    *this = other;
}

Scene& Scene::operator=(const Scene& other) {

    if (this == &other) {
        return *this;
    }
    materials_ = other.materials_;
    spheres_ = other.spheres_;
    lights_ = other.lights_;
    objects_ = other.objects_;

    for (auto& obj : objects_) {
        if (obj.material) {
            auto it = materials_.find(obj.material->name);
            if (it != materials_.end()) {
                obj.material = &it->second;
            } else {
                obj.material = nullptr;
            }
        }
    }
    for (auto& sph : spheres_) {
        if (sph.material) {
            auto it = materials_.find(sph.material->name);
            if (it != materials_.end()) {
                sph.material = &it->second;
            } else {
                sph.material = nullptr;
            }
        }
    }
    return *this;
}

Scene::Scene(Scene&& other) noexcept {

    *this = std::move(other);
}

Scene& Scene::operator=(Scene&& other) noexcept {

    if (this == &other) {
        return *this;
    }
    materials_ = std::move(other.materials_);
    spheres_ = std::move(other.spheres_);
    lights_ = std::move(other.lights_);
    objects_ = std::move(other.objects_);

    for (auto& obj : objects_) {
        if (obj.material) {
            auto it = materials_.find(obj.material->name);
            if (it != materials_.end()) {
                obj.material = &it->second;
            } else {
                obj.material = nullptr;
            }
        }
    }
    for (auto& sph : spheres_) {
        if (sph.material) {
            auto it = materials_.find(sph.material->name);
            if (it != materials_.end()) {
                sph.material = &it->second;
            } else {
                sph.material = nullptr;
            }
        }
    }
    return *this;
}

const std::vector<Object>& Scene::GetObjects() const {

    return objects_;
}

const std::vector<SphereObject>& Scene::GetSphereObjects() const {

    return spheres_;
}

const std::vector<Light>& Scene::GetLights() const {

    return lights_;
}

const std::unordered_map<std::string, Material>& Scene::GetMaterials() const {

    return materials_;
}

std::unordered_map<std::string, Material> ReadMaterials(const std::filesystem::path& path) {

    std::unordered_map<std::string, Material> materials;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open material file: " + path.string());
    }

    std::string line;
    std::string current_material;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "newmtl") {
            ss >> current_material;
            materials[current_material].name = current_material;
            materials[current_material].albedo = {1, 0, 0};
            materials[current_material].specular_exponent = 1;
            materials[current_material].refraction_index = 1;
        } else if (type == "Ka") {
            ss >> materials[current_material].ambient_color[0] >>
                materials[current_material].ambient_color[1] >>
                materials[current_material].ambient_color[2];
        } else if (type == "Kd") {
            ss >> materials[current_material].diffuse_color[0] >>
                materials[current_material].diffuse_color[1] >>
                materials[current_material].diffuse_color[2];
        } else if (type == "Ks") {
            ss >> materials[current_material].specular_color[0] >>
                materials[current_material].specular_color[1] >>
                materials[current_material].specular_color[2];
        } else if (type == "Ke") {
            ss >> materials[current_material].intensity[0] >>
                materials[current_material].intensity[1] >>
                materials[current_material].intensity[2];
        } else if (type == "Ns") {
            ss >> materials[current_material].specular_exponent;
        } else if (type == "Ni") {
            ss >> materials[current_material].refraction_index;
        } else if (type == "al") {
            ss >> materials[current_material].albedo[0] >> materials[current_material].albedo[1] >>
                materials[current_material].albedo[2];
        }
    }
    return materials;
}

void Scene::Load(const std::filesystem::path& path) {

    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open scene file: " + path.string());
    }

    std::vector<Vector> v_pool;
    std::vector<Vector> vn_pool;
    const Material* current_material = nullptr;

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "mtllib") {
            std::string filename;
            ss >> filename;
            auto new_mats = ReadMaterials(path.parent_path() / filename);
            for (const auto& [k, v] : new_mats) {
                materials_[k] = v;
            }
        } else if (type == "usemtl") {
            std::string mat_name;
            ss >> mat_name;
            if (materials_.contains(mat_name)) {
                current_material = &materials_.at(mat_name);
            } else {
                current_material = nullptr;
            }
        } else if (type == "v") {
            double x;
            double y;
            double z;
            if (!(ss >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) ||
                !std::isfinite(z)) {
                throw std::runtime_error("Invalid vertex or normal: " + line);
            }
            v_pool.push_back(Vector(x, y, z));
        } else if (type == "vn") {
            double x;
            double y;
            double z;
            if (!(ss >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) ||
                !std::isfinite(z)) {
                throw std::runtime_error("Invalid vertex or normal: " + line);
            }
            vn_pool.push_back(Vector(x, y, z));
        } else if (type == "f") {
            std::string token;
            std::vector<int> v_idx;
            std::vector<int> vn_idx;

            while (ss >> token) {
                if (token[0] == '#') {
                    break;
                }
                size_t first = token.find('/');
                size_t second = token.find('/', first != std::string::npos ? first + 1 : 0);

                int v = 0;
                int vn = 0;
                bool has_vn = false;

                if (first == std::string::npos) {
                    v = std::stoi(token);
                } else if (second == std::string::npos) {
                    v = std::stoi(token.substr(0, first));
                } else {
                    v = std::stoi(token.substr(0, first));
                    vn = std::stoi(token.substr(second + 1));
                    has_vn = true;
                }

                if (v == 0 || v > static_cast<int>(v_pool.size()) ||
                    v < -static_cast<int>(v_pool.size())) {
                    throw std::runtime_error("Invalid vertex index: " + token);
                }
                if (has_vn && (vn == 0 || vn > static_cast<int>(vn_pool.size()) ||
                               vn < -static_cast<int>(vn_pool.size()))) {
                    throw std::runtime_error("Invalid normal index: " + token);
                }

                if (v > 0) {
                    v = v - 1;
                } else {
                    v = static_cast<int>(v_pool.size()) + v;
                }
                v_idx.push_back(v);

                if (has_vn) {
                    if (vn > 0) {
                        vn = vn - 1;
                    } else {
                        vn = static_cast<int>(vn_pool.size()) + vn;
                    }
                    vn_idx.push_back(vn);
                } else {
                    vn_idx.push_back(-1);
                }
            }

            if (v_idx.size() < 3) {
                throw std::runtime_error("Face must contain at least three vertices");
            }

            for (size_t i = 1; i + 1 < v_idx.size(); ++i) {
                Object obj{
                    .material = current_material,
                    .polygon = Triangle(v_pool[v_idx[0]], v_pool[v_idx[i]], v_pool[v_idx[i + 1]]),
                    .normals = {Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0)},
                    .has_normals = {false, false, false}};

                if (vn_idx[0] >= 0 && vn_idx[0] < static_cast<int>(vn_pool.size())) {
                    obj.normals[0] = vn_pool[vn_idx[0]];
                    obj.has_normals[0] = true;
                }
                if (vn_idx[i] >= 0 && vn_idx[i] < static_cast<int>(vn_pool.size())) {
                    obj.normals[1] = vn_pool[vn_idx[i]];
                    obj.has_normals[1] = true;
                }
                if (vn_idx[i + 1] >= 0 && vn_idx[i + 1] < static_cast<int>(vn_pool.size())) {
                    obj.normals[2] = vn_pool[vn_idx[i + 1]];
                    obj.has_normals[2] = true;
                }

                if (!obj.has_normals[0] || !obj.has_normals[1] || !obj.has_normals[2]) {
                    double ax = obj.polygon[1][0] - obj.polygon[0][0];
                    double ay = obj.polygon[1][1] - obj.polygon[0][1];
                    double az = obj.polygon[1][2] - obj.polygon[0][2];
                    double bx = obj.polygon[2][0] - obj.polygon[0][0];
                    double by = obj.polygon[2][1] - obj.polygon[0][1];
                    double bz = obj.polygon[2][2] - obj.polygon[0][2];

                    double nx = ay * bz - az * by;
                    double ny = az * bx - ax * bz;
                    double nz = ax * by - ay * bx;

                    double len = std::sqrt(nx * nx + ny * ny + nz * nz);
                    if (len > 1e-9) {
                        nx /= len;
                        ny /= len;
                        nz /= len;
                    } else {
                        nx = 0;
                        ny = 0;
                        nz = 0;
                    }

                    if (!obj.has_normals[0]) {
                        obj.normals[0] = Vector(nx, ny, nz);
                        obj.has_normals[0] = true;
                    }
                    if (!obj.has_normals[1]) {
                        obj.normals[1] = Vector(nx, ny, nz);
                        obj.has_normals[1] = true;
                    }
                    if (!obj.has_normals[2]) {
                        obj.normals[2] = Vector(nx, ny, nz);
                        obj.has_normals[2] = true;
                    }
                }

                objects_.push_back(obj);
            }
        } else if (type == "S") {
            double x;
            double y;
            double z;
            double r;
            if (!(ss >> x >> y >> z >> r) || !std::isfinite(x) || !std::isfinite(y) ||
                !std::isfinite(z) || !std::isfinite(r) || r <= 0) {
                throw std::runtime_error("Invalid sphere: " + line);
            }
            spheres_.push_back(
                SphereObject{.material = current_material, .sphere = Sphere(Vector(x, y, z), r)});
        } else if (type == "P") {
            double x;
            double y;
            double z;
            double r;
            double g;
            double b;
            if (!(ss >> x >> y >> z >> r >> g >> b) || !std::isfinite(x) ||
                !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(r) ||
                !std::isfinite(g) || !std::isfinite(b)) {
                throw std::runtime_error("Invalid light: " + line);
            }
            lights_.push_back(Light{.position = Vector(x, y, z), .intensity = Vector(r, g, b)});
        }
    }
}

Scene ReadScene(const std::filesystem::path& path) {

    Scene scene;
    scene.Load(path);
    return scene;
}
