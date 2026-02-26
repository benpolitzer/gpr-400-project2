#include <iostream>

#include "rtweekend.h"
#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "box.h"
#include "image_texture.h"
#include "finite_plane.h"


// Ray Tracing in One Weekend Tutorial scene (with minor tweaks)
static hittable_list random_scene()
{
    hittable_list world;

    std::shared_ptr<material> ground_material =
        std::make_shared<lambertian>(color(0.5, 0.5, 0.5));

    world.add(std::make_shared<sphere>(
        point3(0.0, -1000.0, 0.0),
        1000.0,
        ground_material
    ));

    for (int a = -11; a < 11; ++a)
    {
        for (int b = -11; b < 11; ++b)
        {
            double choose_mat = random_double();
            point3 center(
                static_cast<double>(a) + 0.9 * random_double(),
                0.2,
                static_cast<double>(b) + 0.9 * random_double()
            );

            if ((center - point3(4.0, 0.2, 0.0)).length() > 0.9)
            {
                std::shared_ptr<material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // Diffuse
                    color albedo = random_vec3() * random_vec3();
                    sphere_material = std::make_shared<lambertian>(albedo);
                }
                else if (choose_mat < 0.95)
                {
                    // Metal
                    color albedo = random_vec3(0.5, 1.0);
                    double fuzz = random_double(0.0, 0.5);
                    sphere_material = std::make_shared<metal>(albedo, fuzz);
                }
                else
                {
                    // Glass
                    sphere_material = std::make_shared<dielectric>(1.5);
                }

                world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
            }
        }
    }

    std::shared_ptr<material> material1 = std::make_shared<dielectric>(1.5);
    world.add(std::make_shared<sphere>(point3(0.0, 1.0, 0.0), 1.0, material1));

    std::shared_ptr<material> material2 =
        std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(std::make_shared<sphere>(point3(-4.0, 1.0, 0.0), 1.0, material2));

    std::shared_ptr<material> material3 =
        std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<sphere>(point3(4.0, 1.0, 0.0), 1.0, material3));

    return world;
}
// Cornell box test
static hittable_list cornell_room_basic()
{
    hittable_list world;

    std::shared_ptr<material> red = std::make_shared<lambertian>(color(0.65, 0.05, 0.05));
    std::shared_ptr<material> green = std::make_shared<lambertian>(color(0.12, 0.45, 0.15));
    std::shared_ptr<material> white = std::make_shared<lambertian>(color(0.73, 0.73, 0.73));

    std::shared_ptr<material> metal_mat = std::make_shared<metal>(color(0.9, 0.9, 0.9), 0.05);
    std::shared_ptr<material> glass = std::make_shared<dielectric>(1.5);

    // Room bounds
    const double x0 = -1.0;
    const double x1 = 1.0;
    const double y0 = 0.0;
    const double y1 = 2.0;
    const double z0 = -1.0;
    const double z1 = 1.0;

    // Left wall 
    world.add(std::make_shared<finite_plane>(
        point3(x0, y0, z0),
        vec3(0.0, (y1 - y0), 0.0),
        vec3(0.0, 0.0, (z1 - z0)),
        green
    ));

    // Right wall 
    world.add(std::make_shared<finite_plane>(
        point3(x1, y0, z0),
        vec3(0.0, 0.0, (z1 - z0)),
        vec3(0.0, (y1 - y0), 0.0),
        red
    ));

    // Floor 
    world.add(std::make_shared<finite_plane>(
        point3(x0, y0, z0),
        vec3((x1 - x0), 0.0, 0.0),
        vec3(0.0, 0.0, (z1 - z0)),
        white
    ));

    // Ceiling 
    world.add(std::make_shared<finite_plane>(
        point3(x0, y1, z0),
        vec3((x1 - x0), 0.0, 0.0),
        vec3(0.0, 0.0, (z1 - z0)),
        white
    ));

    // Back wall
    world.add(std::make_shared<finite_plane>(
        point3(x0, y0, z1),
        vec3((x1 - x0), 0.0, 0.0),
        vec3(0.0, (y1 - y0), 0.0),
        white
    ));

    std::shared_ptr<material> light = std::make_shared<diffuse_light>(color(6.0, 6.0, 6.0));

    world.add(std::make_shared<finite_plane>(
        point3(-0.3, y1 - 1e-4, -0.3),
        vec3(0.6, 0.0, 0.0),
        vec3(0.0, 0.0, 0.6),
        light
    ));

    point3 c(0.0, 1.0, -0.2);
    world.add(std::make_shared<sphere>(c, 0.35, glass));
    world.add(std::make_shared<sphere>(c, -0.33, glass));

    world.add(std::make_shared<sphere>(
        point3(0.0, 1.0, -0.2),
        0.30,
        metal_mat
    ));

    return world;
}
// Box test
static hittable_list test_box_scene()
{
    hittable_list world;

    std::shared_ptr<material> white = std::make_shared<lambertian>(color(0.73, 0.73, 0.73));
    world.add(std::make_shared<box>(
        point3(-1.0, 0.0, -1.0),
        point3(1.0, 2.0, 1.0),
        white,
        true
    ));

    return world;
}
// Texture-mapping test
static hittable_list earth_scene()
{
    hittable_list world;

    std::shared_ptr<texture> earth_tex = std::make_shared<image_texture>("Textures/earth.jpg");
    std::shared_ptr<material> earth_mat = std::make_shared<lambertian>(earth_tex);

    std::shared_ptr<texture> moon_tex = std::make_shared<image_texture>("Textures/moon.jpg");
    std::shared_ptr<material> moon_mat = std::make_shared<lambertian>(moon_tex);

    world.add(std::make_shared<sphere>(point3(0.0, 0.0, 0.0), 1.0, earth_mat));
    world.add(std::make_shared<sphere>(point3(4.0, 0.0, 7.0), 0.25, moon_mat));

    return world;
}
// Earth and moon texture mapping with cornell box
static hittable_list cornell_with_earth()
{
    hittable_list world;

    std::shared_ptr<material> red = std::make_shared<lambertian>(color(0.65, 0.05, 0.05));
    std::shared_ptr<material> green = std::make_shared<lambertian>(color(0.12, 0.45, 0.15));
    std::shared_ptr<material> white = std::make_shared<lambertian>(color(0.73, 0.73, 0.73));

    std::shared_ptr<material> metal_mat = std::make_shared<metal>(color(0.9, 0.9, 0.9), 0.05);
    std::shared_ptr<material> glass = std::make_shared<dielectric>(1.5);

    // Room bounds
    const double x0 = -1.0;
    const double x1 = 1.0;
    const double y0 = 0.0;
    const double y1 = 2.0;
    const double z0 = -1.0;
    const double z1 = 1.0;

    // Left wall 
    world.add(std::make_shared<finite_plane>(
        point3(x0, y0, z0),
        vec3(0.0, (y1 - y0), 0.0),
        vec3(0.0, 0.0, (z1 - z0)),
        green
    ));

    // Right wall 
    world.add(std::make_shared<finite_plane>(
        point3(x1, y0, z0),
        vec3(0.0, 0.0, (z1 - z0)),
        vec3(0.0, (y1 - y0), 0.0),
        red
    ));

    // Floor 
    world.add(std::make_shared<finite_plane>(
        point3(x0, y0, z0),
        vec3((x1 - x0), 0.0, 0.0),
        vec3(0.0, 0.0, (z1 - z0)),
        white
    ));

    // Ceiling 
    world.add(std::make_shared<finite_plane>(
        point3(x0, y1, z0),
        vec3((x1 - x0), 0.0, 0.0),
        vec3(0.0, 0.0, (z1 - z0)),
        white
    ));

    // Back wall
    world.add(std::make_shared<finite_plane>(
        point3(x0, y0, z1),
        vec3((x1 - x0), 0.0, 0.0),
        vec3(0.0, (y1 - y0), 0.0),
        white
    ));

    std::shared_ptr<material> light = std::make_shared<diffuse_light>(color(6.0, 6.0, 6.0));

    world.add(std::make_shared<finite_plane>(
        point3(-0.3, y1 - 1e-4, -0.3),
        vec3(0.6, 0.0, 0.0),
        vec3(0.0, 0.0, 0.6),
        light
    ));

    world.add(std::make_shared<finite_plane>(
        point3(-0.3, 0.005, -0.3),
        vec3(0.6, 0.0, 0.0),
        vec3(0.0, 0.0, 0.6),
        light
    ));


    std::shared_ptr<texture> earth_tex = std::make_shared<image_texture>("Textures/earth.jpg");
    std::shared_ptr<material> earth_mat = std::make_shared<lambertian>(earth_tex);

    std::shared_ptr<texture> moon_tex = std::make_shared<image_texture>("Textures/moon.jpg");
    std::shared_ptr<material> moon_mat = std::make_shared<lambertian>(moon_tex);

    world.add(std::make_shared<sphere>(point3(0.0, 1.0, 0.0), 0.5, earth_mat));
    world.add(std::make_shared<sphere>(point3(0.75, 1.5, 0.5), 0.1, moon_mat));

    return world;
}