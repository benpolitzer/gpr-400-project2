#pragma once
#include "hittable.h"

class sphere : public hittable 
{
public:
    point3 center;
    double radius;
    shared_ptr<material> mat;

    sphere() : center(), radius(1), mat(nullptr) {}
    sphere(point3 c, double r, shared_ptr<material> m) : center(c), radius(r), mat(std::move(m)) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override 
    {
        vec3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = half_b * half_b - a * c;
        if (discriminant < 0) return false;
        auto sqrtd = std::sqrt(discriminant);

        auto root = (-half_b - sqrtd) / a;
        if (!ray_t.surrounds(root)) 
        {
            root = (-half_b + sqrtd) / a;
            if (!ray_t.surrounds(root)) return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;
        return true;
    }
};
