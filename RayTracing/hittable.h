#pragma once
#include "rtweekend.h"

// Declares the base interface for all renderable objects
    // hit_record structure used to store intersection details

class material;
struct hit_record 
{
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t{};
    bool front_face{};

    double u = 0.0;   
    double v = 0.0; 

    void set_face_normal(const ray& r, const vec3& outward_normal)
    {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable
{
public:
    virtual ~hittable() = default;
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
};
