#pragma once
#include "hittable.h"
// Implements ray-sphere intersection and stores sphere material data
    // computes surface normals (and UVs if enabled) for shading/texture lookup
class sphere : public hittable 
{
public:
    point3 center;
    double radius;
    shared_ptr<material> mat;

    sphere() : center(), radius(1), mat(nullptr) {}
    sphere(point3 c, double r, shared_ptr<material> m) : center(c), radius(r), mat(std::move(m)) {}
    static void get_sphere_uv(const vec3& p, double& u, double& v) {

        const double pi = 3.1415926535897932385;

        double theta = acos(-p.y());
        double phi = atan2(-p.z(), p.x()) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override 
    {
        vec3 oc = r.origin() - center;
        double a = r.direction().length_squared();
        double half_b = dot(oc, r.direction());
        double c = oc.length_squared() - radius * radius;

        double discriminant = half_b * half_b - a * c;
        if (discriminant < 0) return false;
        double sqrtd = std::sqrt(discriminant);

        double root = (-half_b - sqrtd) / a;
        if (!ray_t.surrounds(root)) 
        {
            root = (-half_b + sqrtd) / a;
            if (!ray_t.surrounds(root)) return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);

        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;
        return true;
    }
};
