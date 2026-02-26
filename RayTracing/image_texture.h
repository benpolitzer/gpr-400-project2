#pragma once
#include "texture.h"
#include <iostream>
#include "stb_image.h"
// Loads an image from disk and samples it using UV coordinates
class image_texture : public texture {
public:
    explicit image_texture(const std::string& filename) {
        int n = 0;
        data = stbi_load(filename.c_str(), &width, &height, &n, 3); // force RGB
        bytes_per_scanline = 3 * width;

        if (!data) {
            width = height = 0;
            std::cerr << "ERROR: Could not load image: " << filename << "\n";
        }
    }

    ~image_texture() override {
        if (data) stbi_image_free(data);
    }

    color value(double u, double v, const point3&) const override {
        if (!data) return color(1, 0, 1); // magenta if missing

        u = (u < 0) ? 0 : (u > 1 ? 1 : u);
        v = (v < 0) ? 0 : (v > 1 ? 1 : v);

        int i = static_cast<int>(u * (width - 1));
        int j = static_cast<int>((1.0 - v) * (height - 1)); // flip v

        const unsigned char* pixel = data + j * bytes_per_scanline + i * 3;
        const double scale = 1.0 / 255.0;

        return color(pixel[0] * scale, pixel[1] * scale, pixel[2] * scale);
    }

private:
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    int bytes_per_scanline = 0;
};