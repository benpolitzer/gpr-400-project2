#pragma once
#include "hittable.h"
#include "material.h"
#include "rtweekend.h"

class finite_plane : public hittable {
public:
    finite_plane() = default;

    finite_plane(const point3& p0_in,
        const vec3& u_in,
        const vec3& v_in,
        shared_ptr<material> mat_in)
        : p0(p0_in), u(u_in), v(v_in), mat_(std::move(mat_in))
    {
        n = unit_vector(cross(u, v));
        uu = dot(u, u);
        uv = dot(u, v);
        vv = dot(v, v);
        det = uu * vv - uv * uv;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        const double denom = dot(n, r.direction());
        if (fabs(denom) < 1e-8) return false; // parallel

        const double t = dot(p0 - r.origin(), n) / denom;
        if (!ray_t.contains(t)) return false;

        const point3 p = r.at(t);

        const vec3 w = p - p0;

        if (fabs(det) < 1e-12) return false;

        const double wu = dot(w, u);
        const double wv = dot(w, v);

        const double a = (wu * vv - wv * uv) / det;
        const double b = (wv * uu - wu * uv) / det;

        if (a < 0.0 || a > 1.0 || b < 0.0 || b > 1.0) return false;

        rec.t = t;
        rec.p = p;
        rec.mat = mat_;
        rec.set_face_normal(r, n);

        return true;
    }

private:
    point3 p0;
    vec3 u, v;
    vec3 n;
    shared_ptr<material> mat_;

    double uu = 0.0, uv = 0.0, vv = 0.0, det = 0.0;
};