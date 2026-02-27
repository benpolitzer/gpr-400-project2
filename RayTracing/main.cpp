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
    //hittable_list world = cornell_room_basic();
    hittable_list world = test_capsule();
    camera cam;

    // Image settings
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1080;
    cam.samples_per_pixel = 100;
    cam.max_depth = 10;

    // Camera placement for cornell box
    cam.vfov = 40.0;
    /*cam.lookfrom = point3(0.0, 1.0, -4);
    cam.lookat = point3(0.0, 1.0, 0.0);
    cam.vup = vec3(0.0, 1.0, 0.0);*/

    // Camera placement for test scene
    cam.lookfrom = point3(0, 8, 20);
    cam.lookat = point3(0, 1, 0);
    cam.vup = vec3(0, 1, 0);

    // Depth of field stuff
    cam.defocus_angle = 0.0;
    cam.focus_dist = 6;

    cam.render_normals(world);
    return 0;
}