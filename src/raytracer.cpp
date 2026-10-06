#include "raytracer.h"
#include "geometry.h"
#include "scene.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

struct IntersectionData {
    Vector position;
    Vector normal;
    Vector geom_normal;
    double distance;
    const Material* material;
    bool is_inside;
};

std::optional<IntersectionData> GetClosestIntersection(const Ray& ray, const Scene& scene) {
    std::optional<IntersectionData> closest;
    double min_dist = std::numeric_limits<double>::infinity();

    for (const auto& obj : scene.GetObjects()) {
        auto hit = GetIntersection(ray, obj.polygon);
        if (hit && hit->GetDistance() > 1e-7 && hit->GetDistance() < min_dist) {
            min_dist = hit->GetDistance();

            Vector geom_normal = CrossProduct(obj.polygon[1] - obj.polygon[0],
                                              obj.polygon[2] - obj.polygon[0]);
            geom_normal.Normalize();
            bool is_inside = false;

            if (DotProduct(geom_normal, ray.GetDirection()) > 0) {
                geom_normal *= -1.0;
                is_inside = true;
            }

            Vector shading_normal;
            if (obj.has_normals[0] && obj.has_normals[1] && obj.has_normals[2]) {
                Vector bary = GetBarycentricCoords(obj.polygon, hit->GetPosition());
                shading_normal =
                    obj.normals[0] * bary[0] + obj.normals[1] * bary[1] + obj.normals[2] * bary[2];
                shading_normal.Normalize();

                if (DotProduct(shading_normal, geom_normal) < 0) {
                    shading_normal *= -1.0;
                }
            } else {
                shading_normal = geom_normal;
            }

            closest = {hit->GetPosition(), shading_normal, geom_normal,
                       hit->GetDistance(), obj.material,   is_inside};
        }
    }

    for (const auto& obj : scene.GetSphereObjects()) {
        auto hit = GetIntersection(ray, obj.sphere);
        if (hit && hit->GetDistance() > 1e-7 && hit->GetDistance() < min_dist) {
            min_dist = hit->GetDistance();

            Vector geom_normal = hit->GetPosition() - obj.sphere.GetCenter();
            geom_normal.Normalize();

            bool is_inside = false;
            if (DotProduct(geom_normal, ray.GetDirection()) > 0) {
                geom_normal *= -1.0;
                is_inside = true;
            }
            closest = {hit->GetPosition(), geom_normal,  geom_normal,
                       hit->GetDistance(), obj.material, is_inside};
        }
    }
    return closest;
}

Vector TraceRay(const Ray& ray, const Scene& scene, int depth, const RenderOptions& options) {
    if (depth < 0) {
        return {0, 0, 0};
    }

    auto hit = GetClosestIntersection(ray, scene);
    if (!hit) {
        if (options.mode == RenderMode::kDepth) {
            return {-1, -1, -1};
        }
        return {0, 0, 0};
    }

    if (options.mode == RenderMode::kDepth) {
        return {hit->distance, hit->distance, hit->distance};
    }

    if (options.mode == RenderMode::kNormal) {
        return {hit->normal[0] * 0.5 + 0.5, hit->normal[1] * 0.5 + 0.5, hit->normal[2] * 0.5 + 0.5};
    }

    Material default_material;
    default_material.diffuse_color = {1, 1, 1};
    const Material* mat = hit->material;
    if (!mat) {
        mat = &default_material;
    }

    Vector diff_sum{0, 0, 0};
    Vector spec_sum{0, 0, 0};

    for (const auto& light : scene.GetLights()) {
        Vector light_dir = light.position - hit->position;
        double light_dist = Length(light_dir);
        light_dir.Normalize();

        Ray shadow_ray(hit->position + hit->geom_normal * 1e-5, light_dir);
        auto shadow_hit = GetClosestIntersection(shadow_ray, scene);

        if (!shadow_hit || shadow_hit->distance > light_dist - 1e-5) {
            double diff_dot = DotProduct(light_dir, hit->normal);

            if (diff_dot > 0) {
                diff_sum[0] += light.intensity[0] * diff_dot;
                diff_sum[1] += light.intensity[1] * diff_dot;
                diff_sum[2] += light.intensity[2] * diff_dot;

                Vector reflected_light = Reflect(light_dir * -1.0, hit->normal);
                double spec_dot = DotProduct(reflected_light, ray.GetDirection() * -1.0);
                if (spec_dot > 0) {
                    double spec_factor = std::pow(spec_dot, mat->specular_exponent);
                    spec_sum[0] += light.intensity[0] * spec_factor;
                    spec_sum[1] += light.intensity[1] * spec_factor;
                    spec_sum[2] += light.intensity[2] * spec_factor;
                }
            }
        }
    }

    Vector color{0, 0, 0};
    color[0] = mat->albedo[0] *
                   (mat->diffuse_color[0] * diff_sum[0] + mat->specular_color[0] * spec_sum[0]) +
               mat->ambient_color[0] + mat->intensity[0];
    color[1] = mat->albedo[0] *
                   (mat->diffuse_color[1] * diff_sum[1] + mat->specular_color[1] * spec_sum[1]) +
               mat->ambient_color[1] + mat->intensity[1];
    color[2] = mat->albedo[0] *
                   (mat->diffuse_color[2] * diff_sum[2] + mat->specular_color[2] * spec_sum[2]) +
               mat->ambient_color[2] + mat->intensity[2];

    bool can_reflect = true;
    if (hit->is_inside && mat->albedo[2] > 0) {
        can_reflect = false;
    }

    if (mat->albedo[1] > 0 && can_reflect && depth > 0) {
        Vector refl_dir = Reflect(ray.GetDirection(), hit->normal);
        Ray refl_ray(hit->position + hit->geom_normal * 1e-5, refl_dir);
        Vector refl_color = TraceRay(refl_ray, scene, depth - 1, options);

        color[0] += refl_color[0] * mat->albedo[1];
        color[1] += refl_color[1] * mat->albedo[1];
        color[2] += refl_color[2] * mat->albedo[1];
    }

    if (mat->albedo[2] > 0 && depth > 0) {
        double eta = hit->is_inside ? mat->refraction_index : (1.0 / mat->refraction_index);
        auto refr_dir = Refract(ray.GetDirection(), hit->normal, eta);

        if (refr_dir) {
            Ray refr_ray(hit->position - hit->geom_normal * 1e-5, *refr_dir);
            Vector refr_color = TraceRay(refr_ray, scene, depth - 1, options);

            double transp = hit->is_inside ? 1.0 : mat->albedo[2];
            color[0] += refr_color[0] * transp;
            color[1] += refr_color[1] * transp;
            color[2] += refr_color[2] * transp;
        }
    }

    return color;
}

Image Render(const std::filesystem::path& path, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    Scene scene = ReadScene(path);

    int width = camera_options.screen_width;
    int height = camera_options.screen_height;
    Image image(width, height);
    std::vector<Vector> buffer(width * height);

    Vector forward = camera_options.look_from - camera_options.look_to;
    forward.Normalize();
    Vector tmp_up{0, 1, 0};

    if (std::abs(forward[0]) < 1e-9 && std::abs(forward[2]) < 1e-9) {
        tmp_up = (forward[1] > 0) ? Vector{0, 0, -1} : Vector{0, 0, 1};
    }

    Vector right = CrossProduct(tmp_up, forward);
    right.Normalize();
    Vector up = CrossProduct(forward, right);
    up.Normalize();

    double scale = std::tan(camera_options.fov / 2.0);
    double aspect_ratio = static_cast<double>(width) / height;
    double max_depth = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double pc_x = (2.0 * (x + 0.5) / width - 1.0) * scale * aspect_ratio;
            double pc_y = (1.0 - 2.0 * (y + 0.5) / height) * scale;

            Vector dir = right * pc_x + up * pc_y + forward * -1.0;
            dir.Normalize();
            Ray ray(camera_options.look_from, dir);

            buffer[y * width + x] = TraceRay(ray, scene, render_options.depth, render_options);

            if (render_options.mode == RenderMode::kDepth) {
                double d = buffer[y * width + x][0];
                if (d > 0) {
                    max_depth = std::max(max_depth, d);
                }
            }
        }
    }

    double max_c = 0;
    if (render_options.mode == RenderMode::kFull) {
        for (const auto& v : buffer) {
            max_c = std::max({max_c, v[0], v[1], v[2]});
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Vector v = buffer[y * width + x];

            if (render_options.mode == RenderMode::kDepth) {
                if (v[0] < 0) {
                    image.SetPixel(RGB{255, 255, 255}, y, x);
                } else {
                    int c = static_cast<int>((v[0] / max_depth) * 255.0);
                    image.SetPixel(RGB{c, c, c}, y, x);
                }
            } else if (render_options.mode == RenderMode::kNormal) {
                int r = static_cast<int>(v[0] * 255.0);
                int g = static_cast<int>(v[1] * 255.0);
                int b = static_cast<int>(v[2] * 255.0);
                image.SetPixel(RGB{r, g, b}, y, x);
            } else {
                if (max_c > 0) {
                    v[0] = (v[0] * (1.0 + v[0] / (max_c * max_c))) / (1.0 + v[0]);
                    v[1] = (v[1] * (1.0 + v[1] / (max_c * max_c))) / (1.0 + v[1]);
                    v[2] = (v[2] * (1.0 + v[2] / (max_c * max_c))) / (1.0 + v[2]);
                }

                int r = static_cast<int>(std::pow(v[0], 1.0 / 2.2) * 255.0);
                int g = static_cast<int>(std::pow(v[1], 1.0 / 2.2) * 255.0);
                int b = static_cast<int>(std::pow(v[2], 1.0 / 2.2) * 255.0);

                r = std::clamp(r, 0, 255);
                g = std::clamp(g, 0, 255);
                b = std::clamp(b, 0, 255);

                image.SetPixel(RGB{r, g, b}, y, x);
            }
        }
    }

    return image;
}
// Нормаль для освещения (Shading normal)
