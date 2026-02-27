#pragma once
#include "hittable.h"

// Finite rectangular plane defined by a corner point and two edge vectors
class infinite_plane : public hittable {
public:
    infinite_plane() = default;

    infinite_plane(const point3& point_on_plane,
        const vec3& normal,
        shared_ptr<material> mat)
        : p(point_on_plane),
        n(unit_vector(normal)),
        mat_(std::move(mat))
    {
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        const double denom = dot(n, r.direction());
        if (std::fabs(denom) < 1e-8) {
            return false;
        }

        const double t = dot(p - r.origin(), n) / denom;
        if (!ray_t.contains(t)) {
            return false;
        }

        rec.t = t;
        rec.p = r.at(t);
        rec.normal = n;
        rec.mat = mat_;
        rec.set_face_normal(r, n);

        return true;
    }

private:
    point3 p;
    vec3 n;
    shared_ptr<material> mat_;
};