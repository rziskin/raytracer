#pragma once

#include "image.h"
#include "options/camera_options.h"
#include "options/render_options.h"

#include <filesystem>

Image Render(const std::filesystem::path& path, const CameraOptions& camera_options,
             const RenderOptions& render_options);
