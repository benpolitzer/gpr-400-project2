#pragma once
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>

#include "interval.h"
#include "ray.h"
#include "vec3.h"

using std::make_shared;
using std::shared_ptr;

inline double degrees_to_radians(double degrees) 
{
    return degrees * 3.1415926535897932385 / 180.0;
}

inline double random_double() 
{
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

inline double random_double(double min, double max) 
{
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max) 
{
    return static_cast<int>(random_double(min, max + 1));
}

inline vec3 random_vec3() 
{
    return vec3(random_double(), random_double(), random_double());
}

inline vec3 random_vec3(double min, double max) 
{
    return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

inline vec3 random_in_unit_sphere() 
{
    while (true) 
    {
        auto p = random_vec3(-1, 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

inline vec3 random_unit_vector() 
{
    return unit_vector(random_in_unit_sphere());
}

inline vec3 random_in_unit_disk() 
{
    while (true) 
    {
        auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

inline vec3 reflect(const vec3& v, const vec3& n) 
{
    return v - 2 * dot(v, n) * n;
}

inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) 
{
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}
