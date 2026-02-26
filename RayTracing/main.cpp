#include <iostream>

#include "rtweekend.h"
#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "box.h"
#include "image_texture.h"
#include "finite_plane.h"
#include "scenes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main()
{
    // Target Scene
    hittable_list world = cornell_with_earth();

    camera cam;

    // Image settings
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1920;
    cam.samples_per_pixel = 1000;
    cam.max_depth = 50;

    // Camera placement
    cam.vfov = 40.0;
    cam.lookfrom = point3(0.0, 1.0, -4);
    cam.lookat = point3(0.0, 1.0, 0.0);
    cam.vup = vec3(0.0, 1.0, 0.0);

    // Depth of field stuff
    cam.defocus_angle = 0.0;
    cam.focus_dist = 4.0;

    cam.render(world);
    return 0;
}